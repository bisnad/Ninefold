"""
Imports
"""
import sys
import json
import threading
import time
from pythonosc import dispatcher, osc_server
from pythonosc.udp_client import SimpleUDPClient

"""
OSC Settings
"""
osc_receive_ip = "0.0.0.0"
osc_receive_port = 9008

osc_send_ip = "127.0.0.1"
osc_send_port = 7400

"""
Presets
"""
class PresetStorage:
    def __init__(self):
        self.presets = {}
        
    def load_from_json(self, filepath):
        try:
            with open(filepath, 'r') as f:
                max_json = json.load(f)
                
            if 'pattrstorage' in max_json and 'slots' in max_json['pattrstorage']:
                slots = max_json['pattrstorage']['slots']
                
                for slot_key, slot_content in slots.items():
                    if 'id' in slot_content and 'data' in slot_content:
                        preset_id = int(slot_content['id'])
                        self.presets[preset_id] = {}
                        
                        for param_name, param_val_list in slot_content['data'].items():
                            if isinstance(param_val_list, list) and len(param_val_list) > 0:
                                try:
                                    float_val = float(param_val_list[0])
                                    self.presets[preset_id][param_name] = float_val
                                except (ValueError, TypeError):
                                    pass
            print(f"Successfully loaded {len(self.presets)} presets from {filepath}.")
        except FileNotFoundError:
            print(f"Warning: {filepath} not found. Starting with empty preset storage.")
        except json.JSONDecodeError:
            print(f"Error: {filepath} is not a valid JSON file.")

"""
OSC Messages
"""
class OSCMessageTemplate:
    def __init__(self, address, fixed_string_pars, fixed_int_pars, variable_float_pars):
        self.address = address
        self.fixed_string_pars = fixed_string_pars
        self.fixed_int_pars = fixed_int_pars
        self.variable_float_pars = variable_float_pars

class OSCMessage:
    def __init__(self, address, parameters):
        self.address = address
        self.parameters = parameters
        
class OSCMessageCreator:
    def __init__(self):
        self.messageTemplates = []
        self.dim02active = 1.0
        self.dim35active = 1.0
        self.dim68active = 1.0
        self.integration_timestep = [0.1] * 9
        
    def addTemplate(self, template):
        self.messageTemplates.append(template)
        
    def createMessages(self, preset):
        oscMessages = []
        for messageTemplate in self.messageTemplates:
            oscMessage = self.createMessage(messageTemplate, preset)
            oscMessages.append(oscMessage)
            
        return oscMessages

    def createDimMessage(self):
        oscMessageAddress = "/SetParameter"
        oscMessageParameters = []
        oscMessageParameters.append("swarm")
        oscMessageParameters.append("integration_timestep")
        float_par_values = [0.0] * 9
        float_par_values[0:3] = [x * self.dim02active for x in self.integration_timestep[0:3]]
        float_par_values[3:6] = [x * self.dim35active for x in self.integration_timestep[3:6]]
        float_par_values[6:9] = [x * self.dim68active for x in self.integration_timestep[6:9]]

        oscMessageParameters += float_par_values

        print("oscMessageParameters ", oscMessageParameters)

        return OSCMessage(oscMessageAddress, oscMessageParameters)

    
    def createMessage(self, messageTemplate, preset):
        oscMessageAddress = messageTemplate.address
        oscMessageParameters = []
        
        if messageTemplate.fixed_string_pars is not None:
            for string_par in messageTemplate.fixed_string_pars:
                oscMessageParameters.append(string_par)
        
        if messageTemplate.fixed_int_pars is not None:
            for int_par in messageTemplate.fixed_int_pars:
                oscMessageParameters.append(int_par)
    
        float_par_values = []
        if messageTemplate.variable_float_pars is not None:
            for float_par_name in messageTemplate.variable_float_pars:
                float_par_value = preset.get(float_par_name, 0.0)
                float_par_values.append(float_par_value)
                
        if messageTemplate.fixed_string_pars is not None and len(messageTemplate.fixed_string_pars) > 1:
            param_name = messageTemplate.fixed_string_pars[1]
            
            if param_name == "integration_timestep" and len(float_par_values) >= 8:

                self.integration_timestep = float_par_values[:9].copy()

                float_par_values[0:3] = [x * self.dim02active for x in float_par_values[0:3]]
                float_par_values[3:6] = [x * self.dim35active for x in float_par_values[3:6]]
                float_par_values[6:9] = [x * self.dim68active for x in float_par_values[6:9]]
                
            elif param_name in ["boundaryMirror_lowerBoundary", "boundaryRepulsion_lowerBoundary", "boundaryWrap_lowerBoundary"]:
                if len(float_par_values) >= 18:
                    float_par_values = [float_par_values[2 * d] - float_par_values[2 * d + 1] * 0.5 for d in range(9)]
                    
            elif param_name in ["boundaryMirror_upperBoundary", "boundaryRepulsion_upperBoundary", "boundaryWrap_upperBoundary"]:
                if len(float_par_values) >= 18:
                    float_par_values = [float_par_values[2 * d] + float_par_values[2 * d + 1] * 0.5 for d in range(9)]

        oscMessageParameters += float_par_values
        return OSCMessage(oscMessageAddress, oscMessageParameters)

"""
OSC Receiver
"""
class OSCReceiver:
    def __init__(self, ip: str, port: int, callback=None):
        self.ip = ip
        self.port = port
        self.server = None
        self.server_thread = None
        self.callback = callback
        self._setup_server()

    def _setup_server(self):
        self.dispatcher = dispatcher.Dispatcher()
        self.dispatcher.map("/preset/select", self._preset_select)
        
        # Centralized dispatcher map for swarm preset visibility
        self.dispatcher.map("/swarm/preset/visibility", self._swarm_preset_visibility)
        
        self.dispatcher.map("/dim02/active", self._dim02_active)
        self.dispatcher.map("/dim35/active", self._dim35_active)
        self.dispatcher.map("/dim68/active", self._dim68_active)
        
        self.server = osc_server.BlockingOSCUDPServer(
            (self.ip, self.port), self.dispatcher
        )

    def start(self):
        print(f"Starting OSC receiver on {self.ip}:{self.port}")
        self.server_thread = threading.Thread(target=self.server.serve_forever)
        self.server_thread.daemon = True
        self.server_thread.start()

    def stop(self):
        if self.server:
            print("Stopping OSC receiver...")
            self.server.shutdown()

    def _preset_select(self, addr: str, *args):
        if self.callback:
            self.callback({"type": "preset_select", "value": int(args[0])})

    def _swarm_preset_visibility(self, addr: str, *args):
        if self.callback and len(args) >= 2:
            # Expects two arguments: index (e.g. 0, 1, 2, 3) and visibility value
            self.callback({
                "type": "swarm_preset_visibility", 
                "index": int(args[0]), 
                "value": int(args[1])
            })

    def _dim02_active(self, addr: str, *args):

        print("dim02_active")

        if self.callback:
            self.callback({"type": "dim02_active", "value": float(args[0])})

    def _dim35_active(self, addr: str, *args):

        print("dim35_active")

        if self.callback:
            self.callback({"type": "dim35_active", "value": float(args[0])})

    def _dim68_active(self, addr: str, *args):

        print("dim68_active")

        if self.callback:
            self.callback({"type": "dim68_active", "value": float(args[0])})

"""
Application Controller
"""
class AppController:
    def __init__(self, storage, creator, sender):
        self.storage = storage
        self.creator = creator
        self.sender = sender

    def on_osc_message_received(self, data):
        msg_type = data.get("type")
        val = data.get("value")
        
        if msg_type == "dim02_active":
            self.creator.dim02active = val
            print(f"Updated dim02active: {val}")
            msg = self.creator.createDimMessage()
            self.sender.send(msg)
        elif msg_type == "dim35_active":
            self.creator.dim35active = val
            print(f"Updated dim35active: {val}")
            msg = self.creator.createDimMessage()
            self.sender.send(msg)
        elif msg_type == "dim68_active":
            self.creator.dim68active = val
            print(f"Updated dim68active: {val}")
            msg = self.creator.createDimMessage()
            self.sender.send(msg)
            
        # Unified handler logic for preset visibility
        elif msg_type == "swarm_preset_visibility":
            index = data.get("index")
            msg = OSCMessage("/AssignNeighbors", ["preset_swarm", index, "position", "preset_position", val])
            self.sender.send(msg)
            print(f"Updated swarm_preset_{index}_visibility: {val}")
            
        elif msg_type == "preset_select":
            print(f"Received preset select: {val}")
            if val in self.storage.presets:
                preset = self.storage.presets[val]
                osc_messages = self.creator.createMessages(preset)
                for msg in osc_messages:
                    self.sender.send(msg)
                print(f"Successfully broadcasted {len(osc_messages)} messages for preset ID {val}.")
            else:
                print(f"Warning: Preset ID {val} not found in storage.")

class OSCSender:
    def __init__(self, ip: str, port: int):
        self.osc_sender = SimpleUDPClient(ip, port)
        print(f"OSC sender initialized: {ip}:{port}")

    def send(self, oscMessage):
        try:
            self.osc_sender.send_message(oscMessage.address, oscMessage.parameters)
        except Exception as e:
            print(f"Error sending data: {e}")

"""
Main Application Runner
"""
def build_message_creator():
    creator = OSCMessageCreator()
    creator.addTemplate(OSCMessageTemplate("/SetSimulationRate", None, None, ["sim_rate"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "integration_timestep"], None, ["step_size", "step_size", "step_size", "step_size", "step_size", "step_size", "step_size", "step_size", "step_size"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "acceleration_maxAngularAcceleration"], None, ["turning_amount", "turning_amount", "turning_amount", "turning_amount", "turning_amount", "turning_amount", "turning_amount", "turning_amount", "turning_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryMirror_lowerBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryMirror_upperBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryRepulsion_lowerBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryRepulsion_upperBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryWrap_lowerBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryWrap_upperBoundary"], None, ["boundary_pos0", "boundary_size0", "boundary_pos1", "boundary_size1", "boundary_pos2", "boundary_size2", "boundary_pos3", "boundary_size3", "boundary_pos4", "boundary_size4", "boundary_pos5", "boundary_size5", "boundary_pos6", "boundary_size6", "boundary_pos7", "boundary_size7", "boundary_pos8", "boundary_size8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryMirror_active"], None, ["boundary_mirror_active"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryWrap_active"], None, ["boundary_wrap_active"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "boundaryRepulsion_active"], None, ["boundary_repulsion_active"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "mass"], None, ["mass"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "randomize_range"], None, ["random", "random", "random", "random", "random", "random", "random", "random", "random"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "damping_prefVelocity"], None, ["velocity"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "damping_amount"], None, ["velocity_damping"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "targetPos_target"], None, ["target_pos0", "target_pos1", "target_pos2", "target_pos3", "target_pos4", "target_pos5", "target_pos6", "target_pos7", "target_pos8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "targetPos_amount"], None, ["target_position_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "targetVel_target"], None, ["target_vel0", "target_vel1", "target_vel2", "target_vel3", "target_vel4", "target_vel5", "target_vel6", "target_vel7", "target_vel8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "targetVel_amount"], None, ["target_vel_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "evasion_maxDist"], None, ["evasion_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "evasion_amount"], None, ["evasion_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "cohesion_maxDist"], None, ["cohesion_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "cohesion_amount"], None, ["cohesion_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "alignment_maxDist"], None, ["alignment_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "alignment_amount"], None, ["alignment_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_evasion_maxDist"], None, ["preset_evasion_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_evasion_amount"], None, ["preset_evasion_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_cohesion_maxDist"], None, ["preset_cohesion_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_cohesion_amount"], None, ["preset_cohesion_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_alignment_maxDist"], None, ["preset_alignment_dist"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["swarm", "preset_alignment_amount"], None, ["preset_alignment_amount"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["preset_swarm", "position"], [0], ["preset0_pos0", "preset0_pos1", "preset0_pos2", "preset0_pos3", "preset0_pos4", "preset0_pos5", "preset0_pos6", "preset0_pos7", "preset0_pos8"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["preset_swarm", "position"], [1], ["preset0_pos0[1]", "preset0_pos1[1]", "preset0_pos2[1]", "preset0_pos3[1]", "preset0_pos4[1]", "preset0_pos5[1]", "preset0_pos6[1]", "preset0_pos7[1]", "preset0_pos8[1]"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["preset_swarm", "position"], [2], ["preset0_pos0[2]", "preset0_pos1[2]", "preset0_pos2[2]", "preset0_pos3[2]", "preset0_pos4[2]", "preset0_pos5[2]", "preset0_pos6[2]", "preset0_pos7[2]", "preset0_pos8[2]"]))
    creator.addTemplate(OSCMessageTemplate("/SetParameter", ["preset_swarm", "position"], [3], ["preset0_pos0[3]", "preset0_pos1[3]", "preset0_pos2[3]", "preset0_pos3[3]", "preset0_pos4[3]", "preset0_pos5[3]", "preset0_pos6[3]", "preset0_pos7[3]", "preset0_pos8[3]"]))

    return creator

if __name__ == "__main__":
    presetStorage = PresetStorage()
    presetStorage.load_from_json('swarm_control.json')
    
    oscMessageCreator = build_message_creator()
    oscSender = OSCSender(osc_send_ip, osc_send_port)
    
    controller = AppController(presetStorage, oscMessageCreator, oscSender)
    
    oscReceiver = OSCReceiver(osc_receive_ip, osc_receive_port, callback=controller.on_osc_message_received)
    oscReceiver.start()
    
    print("\nPress Ctrl-C to quit.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nCtrl-C received. Shutting down...")
    finally:
        oscReceiver.stop()
        sys.exit(0)