# osc_score_pyqt_minimal.py

import sys
import time
import threading
from pythonosc.udp_client import SimpleUDPClient
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QPushButton, QCheckBox
)
from PyQt5.QtCore import QTimer

# ---------- Score Definition ----------

score_progression_auto = True
score_progression_manual_step = False

score = {
    "part1": {
        "time":             [0, 1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
        "flock_preset":     [100, 100, 100, 200, 200, 300, 300, 400, 400, 500, 500, 600, 600, 700, 700, 800, 800, 900, 900, 900],
        "presetflock_vis":  [ [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [1, 0, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1], [0, 0, 0, 0], [0, 1, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 1], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0] ],
        "synth1_active":    [ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
        "synth2_active":    [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        "synth2_preset":    [ 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 8]
    },
    "part2": {
        "time":    [0, 1, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360, 390, 420, 450, 480],
        "flock_preset":   [100, 100, 100, 200, 200, 300, 300, 400, 400, 500, 500, 600, 600, 700, 700, 800, 800, 900, 900, 900],
        "presetflock_vis": [ [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [1, 0, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1], [0, 0, 0, 0], [0, 1, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 1], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0] ],
        "synth1_active":   [ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
        "synth2_active":   [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        "synth2_preset":   [ 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 8]
    },
    "part_sequence": ["part1", "part2"]
}

# ---------- OSC Settings ----------

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

# ---------- OSC Functions ----------

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

# ---------- Score Playback ----------

current_score_part = 0
current_score_step = 0
score_update_interval = 0.01
stop_event = threading.Event()
score_thread = None

def run_score_task():
    global stop_event, current_score_part, current_score_step, score_progression_manual_step

    start_time = time.perf_counter()
    while not stop_event.is_set():
        elapsed = time.perf_counter() - start_time
        part_name = score["part_sequence"][current_score_part]
        step_time = score[part_name]["time"][current_score_step]

        if (score_progression_auto and elapsed > step_time) or \
           (not score_progression_auto and score_progression_manual_step):
            score_progression_manual_step = False
            print(f"Part: {part_name}, Step: {current_score_step}, Time: {step_time}, Elapsed: {elapsed:.2f}")
            score_osc_send(part_name, current_score_step)

            current_score_step += 1
            if current_score_step >= len(score[part_name]["time"]):
                current_score_step = 0
                current_score_part += 1
                start_time = time.perf_counter()
                if current_score_part >= len(score["part_sequence"]):
                    current_score_part = 0
                    #stop_event.set()
        time.sleep(score_update_interval)

def start_score_task():
    global stop_event, score_thread
    stop_event.clear()
    score_thread = threading.Thread(target=run_score_task)
    score_thread.start()

# ---------- GUI ----------

class ScoreGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('OSC Score Control')
        self.layout = QVBoxLayout()

        self.display_labels = {}
        for label in ['Score Part', 'Step Index', 'Time', 'Flock Preset', 'Preset Vis', 'Synth1 Active', 'Synth2 Active', 'Synth2 Preset']:
            lbl = QLabel(f"{label}:")
            self.display_labels[label] = lbl
            self.layout.addWidget(lbl)

        # Controls
        self.start_btn = QPushButton('Start')
        self.stop_btn = QPushButton('Stop')
        self.reset_btn = QPushButton('Reset')
        self.auto_checkbox = QCheckBox('Automated Progression')
        self.step_btn = QPushButton('Step (Manual)')

        self.start_btn.clicked.connect(self.on_start)
        self.stop_btn.clicked.connect(self.on_stop)
        self.reset_btn.clicked.connect(self.on_reset)
        self.auto_checkbox.setChecked(score_progression_auto)
        self.auto_checkbox.stateChanged.connect(self.on_toggle_auto)
        self.step_btn.clicked.connect(self.on_step)

        ctrl_layout = QHBoxLayout()
        for btn in [self.start_btn, self.stop_btn, self.reset_btn, self.auto_checkbox, self.step_btn]:
            ctrl_layout.addWidget(btn)
        self.layout.addLayout(ctrl_layout)

        self.setLayout(self.layout)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_display)
        self.timer.start(150)

    def on_start(self):
        global score_thread
        if not score_thread or not score_thread.is_alive():
            start_score_task()

    def on_stop(self):
        global stop_event, score_thread
        stop_event.set()
        if score_thread and score_thread.is_alive():
            score_thread.join()
            score_thread = None

    def on_reset(self):
        global stop_event, current_score_part, current_score_step, score_thread
        stop_event.set()
        if score_thread and score_thread.is_alive():
            score_thread.join()
            score_thread = None
        current_score_part = 0
        current_score_step = 0

    def on_toggle_auto(self):
        global score_progression_auto
        score_progression_auto = self.auto_checkbox.isChecked()

    def on_step(self):
        global score_progression_manual_step
        score_progression_manual_step = True

    def update_display(self):
        global current_score_part, current_score_step

        seq = score["part_sequence"]
        part_name = seq[current_score_part] if current_score_part < len(seq) else "(finished)"
        part = score.get(part_name, {})
        idx = current_score_step

        get_val = lambda key: part.get(key, [""])[idx] if key in part and idx < len(part[key]) else ""

        self.display_labels['Score Part'].setText(f"Score Part: {part_name}")
        self.display_labels['Step Index'].setText(f"Step Index: {idx}")
        self.display_labels['Time'].setText(f"Time: {get_val('time')}")
        self.display_labels['Flock Preset'].setText(f"Flock Preset: {get_val('flock_preset')}")
        self.display_labels['Preset Vis'].setText(f"Preset Vis: {get_val('preset_flock_vis')}")
        self.display_labels['Synth1 Active'].setText(f"Synth1 Active: {get_val('synth1_active')}")
        self.display_labels['Synth2 Active'].setText(f"Synth2 Active: {get_val('synth2_active')}")
        self.display_labels['Synth2 Preset'].setText(f"Synth2 Preset: {get_val('synth2_preset')}")

# ---------- Main ----------

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ScoreGUI()
    window.show()
    sys.exit(app.exec_())
