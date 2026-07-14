#!/bin/bash

# Ninefold Software Home
Ninefold_Home="$HOME/Projects/Ninefold/Software"

# Activate Ninefold conda environment
CONDA_PATH="$HOME/miniconda3"
CONDA_ENV="Ninefold"
source "$CONDA_PATH/bin/activate" "$CONDA_ENV"
echo "Conda environment $CONDA_ENV activated"
sleep 1

# Start Button Script
cd "$Ninefold_Home/Python/button"
python button.py &
echo "Button Script Started"
sleep 1

# Start Swarm Simulation
cd "$Ninefold_Home/Swarms/bin"
./Swarms &
echo "Swarm Simulation Started"
sleep 1

# Start Swarm Control Script
cd "$Ninefold_Home/Swarms/controls"
python swarm_control.py &
echo "Swarm Control Script Started"
sleep 1

# Start Swarm Visualisation
cd "$Ninefold_Home/Swarms_Display_v2/bin"
./Swarms_Display_D02 &
echo "Swarm Visualisation Started"
sleep 1

# Start Swarm Visualisation Control Script
cd "$Ninefold_Home/Swarms/controls"
python swarm_vis_control_D02.py &
echo "Swarm Visualisation Control Script Started"
sleep 10

# Start OSC Score Script
cd "$Ninefold_Home/Python/score"
python osc_score.py &
echo "OSC Score Started"
sleep 1



