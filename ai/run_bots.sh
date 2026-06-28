#!/bin/bash

# Configuration variables
PORT=8080
MODEL="training/models/zappy_level2_v2"

# Loop through the list of teams
for team in "team01" "team02"; do

    # For each team, launch 2 bots
    for i in {1..2}; do
        python3 src/main.py -p $PORT -n "$team" -ip 127.0.0.1 --ai --model "$MODEL" &
    done

done

echo "¡10 bots running model '$MODEL' on port $PORT launched in the background!"
echo "AS the bots are running in the background, do 'pkill -f \"src/main.py'\" to stop them"