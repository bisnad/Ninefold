"""
OSC Score
"""

"""
Imports
"""

import time
import threading
import json
import numpy as np
from pythonosc.udp_client import SimpleUDPClient

"""
Score Settings
"""

score_progression_auto = True
score_progression_manual_step = False

score = {
    "part1": {
        "time":             [0, 1, 2, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 88, 92],
        "flock_preset":     [100, 100, 100, 100, 200, 200, 300, 300, 400, 400, 500, 500, 600, 600, 700, 700, 800, 800, 900, 900, 900, 900],
        "presetflock_vis":  [ [0, 0, 0, 0,], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [1, 0, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1], [0, 0, 0, 0], [0, 1, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 1], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0] ],
        "synth1_active":    [ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
        "synth2_active":    [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        "synth2_preset":    [ 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 10]
    },
    "part2": {
        "time":             [0, 1, 2, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360, 390, 420, 450, 480, 510, 513, 517],
        "flock_preset":     [100, 100, 100, 100, 200, 200, 300, 300, 400, 400, 500, 500, 600, 600, 700, 700, 800, 800, 900, 900, 900, 900],
        "presetflock_vis":  [ [0, 0, 0, 0,], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [1, 0, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1], [0, 0, 0, 0], [0, 1, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 1], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0] ],
        "synth1_active":    [ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
        "synth2_active":    [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        "synth2_preset":    [ 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 10]
    },
    "part_sequence": ["part1", "part2"]
}

"""
OSC Settings
"""

osc_settings = {
    "flock_preset": {
        "ip": "192.168.1.101",
        "port": 9008,
        "ma": "/preset/select"
    },
    "presetflock_vis": {
        "ip": "192.168.1.101",
        "port": 9008,
        "ma": "/swarm/preset/visibility"
    },
    "synth1_active": {
        "ip": "192.168.1.103",
        "port": 9005,
        "ma": "/synth1"
    },
    "synth2_active": {
        "ip": "192.168.1.104",
        "port": 9005,
        "ma": "/synth2"
    },
    "synth2_preset": {
        "ip": "192.168.1.104",
        "port": 9006,
        "ma": "/preset"
    }
}

"""
OSC Functions
"""

def setup_osc_senders():
    for key in osc_settings:
        settings = osc_settings[key]
        settings["sender"] = SimpleUDPClient(settings["ip"], settings["port"])

def score_osc_send(score_part_name, score_step):
    part = score[score_part_name]

    # Flock preset
    preset_nr = part["flock_preset"][score_step]
    if preset_nr != -1:
        print("flock_preset ", preset_nr)
        osc_settings["flock_preset"]["sender"].send_message(osc_settings["flock_preset"]["ma"], [preset_nr])

    # Preset Flock Visibility
    presets_vis = part["presetflock_vis"][score_step]
    for preset_index, preset_vis in enumerate(presets_vis):
        if preset_vis != -1:
            print("presetflock_vis ", preset_index, " ", preset_vis)
            osc_settings["presetflock_vis"]["sender"].send_message(osc_settings["presetflock_vis"]["ma"], [preset_index, preset_vis])

    # Synth 1 active
    synth1_active = part["synth1_active"][score_step]
    print("synth1_active ", synth1_active)
    osc_settings["synth1_active"]["sender"].send_message(osc_settings["synth1_active"]["ma"], [synth1_active])

    # Synth 2 active
    synth2_active = part["synth2_active"][score_step]
    print("synth2_active ", synth2_active)
    osc_settings["synth2_active"]["sender"].send_message(osc_settings["synth2_active"]["ma"], [synth2_active])

    # Synth 2 preset
    synth2_preset = part["synth2_preset"][score_step]
    if synth2_preset != -1:
        print("synth2_preset ", synth2_preset)
        osc_settings["synth2_preset"]["sender"].send_message(osc_settings["synth2_preset"]["ma"], [synth2_preset])

setup_osc_senders()

"""
Score Functions
"""

current_score_part = 0
current_score_step = 0 

"""
Score Progression
"""

score_update_interval = 0.01 # seconds

stop_event = threading.Event()

def run_score_task():
    
    global stop_event
    global score_thread
    global current_score_part
    global current_score_step
    global score_progression_manual_step
    
    start_time = time.perf_counter()
    
    while not stop_event.is_set():
        
        elapsed = time.perf_counter() - start_time
        
        score_part_name = score["part_sequence"][current_score_part]
        score_step_time = score[score_part_name]["time"][current_score_step]
        
        if (score_progression_auto == True and elapsed > score_step_time) or (score_progression_auto == False and score_progression_manual_step == True):
           
            score_progression_manual_step = False
            
            print("part ", score_part_name, " step ", current_score_step, " step_time",score_step_time, " elapsed ",  elapsed)
            #print("elapsed ", elapsed, " score_step_time ", score_step_time, " current_score_step ", current_score_step)
            
            score_osc_send(score_part_name, current_score_step)

            current_score_step += 1
            
            if current_score_step >= len(score[score_part_name]["time"]):
                
                current_score_step = 0
                current_score_part += 1
                start_time = time.perf_counter()
                
                if current_score_part >= len(score["part_sequence"]):

                    current_score_part = 0                  
                    #stop_event.set()
                    
        
        # Wait until 10 ms have passed since loop start
        sleep_duration = max(0, score_update_interval - elapsed)
        time.sleep(sleep_duration)

def start_score_task():
    score_thread = threading.Thread(target=run_score_task, args=())
    score_thread.start()
    return score_thread

score_thread = start_score_task()




"""
score_thread.join()
"""
