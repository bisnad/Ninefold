import serial
import time
from pythonosc.udp_client import SimpleUDPClient

"""
Serial Settings
"""

serial_port = '/dev/ttyACM0'
serial_baud_rate = 9600


"""
OSC Settings
"""

#osc_send_ip = "127.0.0.1"
osc_send_swarmcontrol_ip = "192.168.1.101"
osc_send_swarmcontrol_port = 9008
osc_send_philippesynth_control_ip = "192.168.1.103"
osc_send_philippesynth_control_port = 9008
osc_send_ephraimsynth_control_ip = "192.168.1.104"
osc_send_ephraimsynth_control_port = 9008

try:

    # create OSC senders
    osc_sender_swarmcontrol = SimpleUDPClient(osc_send_swarmcontrol_ip, osc_send_swarmcontrol_port)
    osc_sender_philippesynth = SimpleUDPClient(osc_send_philippesynth_control_ip, osc_send_philippesynth_control_port)
    osc_sender_ephraimsynth = SimpleUDPClient(osc_send_ephraimsynth_control_ip, osc_send_ephraimsynth_control_port)

    # Open the serial port with a timeout so readline() doesn't block forever
    ser = serial.Serial(serial_port, serial_baud_rate, timeout=1)
    print(f"Listening on {serial_port}...\nPress Ctrl+C to stop.")

    # Allow a brief moment for the connection to establish
    time.sleep(2) 

    while True:
        # Check if there is data waiting in the buffer
        if ser.in_waiting > 0:
            # Read the line, decode it from bytes to a string, and strip the newline
            message = ser.readline().decode('utf-8').strip()
            
            # The message should look like "1 1" or "1 0" based on your Arduino code
            if message:
                parts = message.split(' ')
                if len(parts) == 2:
                    button_id = int(parts[0])
                    button_state = int(parts[1])
                    button_active = "Active" if button_state == 1 else "Inactive"
                    print(f"Received -> Button ID: {button_id} | State: {button_active}")

                    if button_id == 1:
                        osc_sender_swarmcontrol.send_message("/dim02/active", [button_state])
                        osc_sender_philippesynth.send_message("/dim02/active", [button_state])
                        osc_sender_ephraimsynth.send_message("/dim02/active", [button_state])
                    elif button_id == 2:
                        osc_sender_swarmcontrol.send_message("/dim35/active", [button_state])
                        osc_sender_philippesynth.send_message("/dim35/active", [button_state])
                        osc_sender_ephraimsynth.send_message("/dim35/active", [button_state])
                    elif button_id == 3:
                        osc_sender_swarmcontrol.send_message("/dim68/active", [button_state])
                        osc_sender_philippesynth.send_message("/dim68/active", [button_state])
                        osc_sender_ephraimsynth.send_message("/dim68/active", [button_state])

                    #osc_sender.send_message("/button/{}".format(int(button_id)), [int(button_state)])

                else:
                    # In case of garbled data during initialization
                    print(f"Raw message: {message}")

except serial.SerialException as e:
    print(f"Error opening or reading from {serial_port}: {e}")
except KeyboardInterrupt:
    print("\nExiting program...")
finally:
    # Ensure the port is always closed gracefully
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.")