"""
Imports
"""
import sys
import math
import json
import threading
import time
from pythonosc import dispatcher, osc_server
from pythonosc.udp_client import SimpleUDPClient

"""
OSC Settings
"""
osc_receive_ip = "0.0.0.0"
osc_receive_port = 9009

osc_send_ip = "192.168.1.101"
osc_send_port = 11002

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
Euler to Quaternion (jit.euler2quat style)
"""

def euler2quat(x, y, z, in_degrees=False):
    """
    Convert Euler X/Y/Z angles to a quaternion.
    Matches jit.euler2quat ordering: [X, Y, Z, W]
    """
    if in_degrees:
        x = math.radians(x)
        y = math.radians(y)
        z = math.radians(z)
        
    # Precalculate trigonometric values
    cy = math.cos(z * 0.5)
    sy = math.sin(z * 0.5)
    cp = math.cos(y * 0.5)
    sp = math.sin(y * 0.5)
    cr = math.cos(x * 0.5)
    sr = math.sin(x * 0.5)

    # Calculate quaternion components
    q_x = sr * cp * cy - cr * sp * sy
    q_y = cr * sp * cy + sr * cp * sy
    q_z = cr * cp * sy - sr * sp * cy
    q_w = cr * cp * cy + sr * sp * sy

    return [q_x, q_y, q_z, q_w]

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
        
    def addTemplate(self, template):
        self.messageTemplates.append(template)
        
    def createMessages(self, preset):
        oscMessages = []
        for messageTemplate in self.messageTemplates:
            oscMessage = self.createMessage(messageTemplate, preset)
            oscMessages.append(oscMessage)
            
        return oscMessages
    
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
                

        if oscMessageAddress == "/DisplayOrientation":
            float_par_values = euler2quat(float_par_values[0], float_par_values[1], float_par_values[2], in_degrees=False)
        if oscMessageAddress == "/TrailLength":
            float_par_values = [ int(float_par_values[0]) ]


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

"""
Application Controller
"""
class AppController:
    def __init__(self, storage, creator, sender):
        self.storage = storage
        self.creator = creator
        self.sender = sender

    def on_osc_message_received(self, data):


        print("data ", data)

        msg_type = data.get("type")
        val = data.get("value")
            
        if msg_type == "preset_select":
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
    creator.addTemplate(OSCMessageTemplate("/DisplayPosition", None, None, ["display_posX", "display_posY", "display_posZ"]))
    creator.addTemplate(OSCMessageTemplate("/DisplayOrientation", None, None, ["display_rotX", "display_rotY", "display_rotZ"]))
    creator.addTemplate(OSCMessageTemplate("/DisplayZoom", None, None, ["display_zoom"]))
    creator.addTemplate(OSCMessageTemplate("/AgentColor", ["preset_swarm"], None, ["preset_swarm_agent_colorR", "preset_swarm_agent_colorG", "preset_swarm_agent_colorB", "preset_swarm_agent_colorA"]))
    creator.addTemplate(OSCMessageTemplate("/AgentScale", ["preset_swarm"], None, ["preset_swarm_agent_scaleX", "preset_swarm_agent_scaleY", "preset_swarm_agent_scaleZ"]))
    creator.addTemplate(OSCMessageTemplate("/AgentColor", ["swarm"], None, ["swarm_agent_colorR", "swarm_agent_colorG", "swarm_agent_colorB", "swarm_agent_colorA"]))
    creator.addTemplate(OSCMessageTemplate("/AgentScale", ["swarm"], None, ["swarm_agent_scaleX", "swarm_agent_scaleY", "swarm_agent_scaleZ"]))
    creator.addTemplate(OSCMessageTemplate("/TrailColor", ["swarm"], None, ["swarm_trail_colorR", "swarm_trail_colorG", "swarm_trail_colorB", "swarm_trail_colorA"]))
    creator.addTemplate(OSCMessageTemplate("/TrailDecay", ["swarm"], None, ["swarm_trail_decay"]))
    creator.addTemplate(OSCMessageTemplate("/TrailLength", ["swarm"], None, ["swarm_trail_length"]))
    creator.addTemplate(OSCMessageTemplate("/SpaceColor", ["spacegrid"], None, ["space_spacegrid_colorR", "space_spacegrid_colorG", "space_spacegrid_colorB", "space_spacegrid_colorA"]))
    creator.addTemplate(OSCMessageTemplate("/SpaceColor", ["preset_position"], None, ["space_presetposition_colorR", "space_presetposition_colorRG", "space_presetposition_colorB", "space_presetposition_colorA"]))
    creator.addTemplate(OSCMessageTemplate("/SpaceColor", ["agent_position"], None, ["space_agentposition_colorR", "space_agentposition_colorG", "space_agentposition_colorB", "space_agentposition_colorA"]))

    return creator

if __name__ == "__main__":
    presetStorage = PresetStorage()
    presetStorage.load_from_json('swarm_vis_control.json')
    
    oscMessageCreator = build_message_creator()
    oscSender = OSCSender(osc_send_ip, osc_send_port)
    
    controller = AppController(presetStorage, oscMessageCreator, oscSender)
    
    oscReceiver = OSCReceiver(osc_receive_ip, osc_receive_port, callback=controller.on_osc_message_received)
    oscReceiver.start()
    

    # select start preset 
    controller.on_osc_message_received({'type': 'preset_select', 'value': 2})
    # oscMessageCreator.createMessages(1)
    # {'type': 'preset_select', 'value': 2}

    print("\nPress Ctrl-C to quit.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nCtrl-C received. Shutting down...")
    finally:
        oscReceiver.stop()
        sys.exit(0)

def euler2quat(x, y, z, in_degrees=False):
    """
    Convert Euler X/Y/Z angles to a quaternion.
    Matches jit.euler2quat ordering: [X, Y, Z, W]
    """
    if in_degrees:
        x = math.radians(x)
        y = math.radians(y)
        z = math.radians(z)
        
    # Precalculate trigonometric values
    cy = math.cos(z * 0.5)
    sy = math.sin(z * 0.5)
    cp = math.cos(y * 0.5)
    sp = math.sin(y * 0.5)
    cr = math.cos(x * 0.5)
    sr = math.sin(x * 0.5)

    # Calculate quaternion components
    q_x = sr * cp * cy - cr * sp * sy
    q_y = cr * sp * cy + sr * cp * sy
    q_z = cr * cp * sy - sr * sp * cy
    q_w = cr * cp * cy + sr * sp * sy

    return [q_x, q_y, q_z, q_w]