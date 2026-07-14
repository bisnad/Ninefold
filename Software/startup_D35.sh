#!/bin/bash

# Ninefold Software Home
Ninefold_Home="$HOME/Projects/Ninefold/Software"

# Start Synth
cd "/usr/bin"
./scide &
echo "SuperCollider started"
sleep 10

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

# Start Swarm Visualisation
cd "$Ninefold_Home/Swarms_Display_v2/bin"
./Swarms_Display_D35 &
echo "Swarm Visualisation Started"
sleep 1

# Start Swarm Visualisation Control Script
cd "$Ninefold_Home/Swarms/controls"
python swarm_vis_control_D35.py &
echo "Swarm Visualisation Control Script Started"
sleep 1



