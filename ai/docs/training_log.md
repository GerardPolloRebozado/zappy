# Zappy AI Training Log

This file acts as a lab notebook to document the parameter tweaks, learning curves, and results of training runs.

## Run #1
- **Base Model**: `Fresh`
- **Output Model Name**: `zappy_survival_v1`
- **Timesteps Run**: `200,000`
- **Real-World Duration**: `1m 50s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 200000 -f 1000 -m zappy_survival_v1
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_survival_v1 --teams team01 --players 1 --width 10 --height 10
```
- **Average Level Achieved**: `1.80`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `84.00`
- **Max Turns Survived**: `102`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_134417_zappy_survival_v1_r1.md](../training/results/eval_20260627_134417_zappy_survival_v1_r1.md)*

#### Observations (when running game with gui)
2 game runs performed
- *Key observations*
  - The player would not take any stones from the ground
  - The player manage to evolve to level 2 when the stone needed was already on the tile. 
  - Player would ignore food on the tiles unless it's life bar fell bellow a certain mark
- *Did it exhibit any loop behaviors or stuck states?*
  - Player would only walk forward. This can be explained by the fact that in level 1 the player can only look one tile ahead, because in this phase the AI is not  
- *Adjustments needed for the next run:*
  - Phase 1 is complete and successful. The agent has mastered basic survival and hunger control. 
  - In Phase 2, we introduce stones needed for elevation, which award a massive +4.0 points (compared to the tiny +0.1 for walking forward). The agent will quickly learn that it cannot get these high rewards by just walking in a straight line; it will be forced to turn, look, and steer towards the stones.

## Run #2
- **Base Model**: `Loaded from zappy_survival_v1`
- **Output Model Name**: `zappy_level2_v1`
- **Timesteps Run**: `300,000`
- **Real-World Duration**: `1m 47s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 300000 -f 1000 -m zappy_level2_v1 -l zappy_survival_v1 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_level2_v1 --teams team01 --players 1 --width 12 --height 12
```
- **Average Level Achieved**: `1`
- **Max Level Achieved**: `1`
- **Average Turns Survived**: `139`
- **Max Turns Survived**: `159`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_134637_zappy_level2_v1_r1.md](../training/results/eval_20260627_134637_zappy_level2_v1_r1.md)*

#### Observations (when running game with gui)
3 game runs performed
- *Key observations*
  - Agent would take food more frequently
  - Agent did pick up Linemate at some point. It died not use it or evolve at any point
  - The agent did perform a turn at some point, but then kept only moving forward
- *Did it exhibit any loop behaviors or stuck states?*
  - Agent still moves forward constantly 
- *Adjustments needed for the next run:*
  - We will continue training Phase 2 for longer. Now that the AI has discovered the stones and started turning, it needs more experience to reinforce and complete the sequence.
  - Make a new run with a training cycle of 500,000 timesteps, loading the current `zappy_level2_v1` model to fine-tune it

## Run #3
- **Base Model**: `Loaded from zappy_level2_v1`
- **Output Model Name**: `zappy_level2_v1`
- **Timesteps Run**: `500,000`
- **Real-World Duration**: `2m 52s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 500000 -f 1000 -m zappy_level2_v1 -l zappy_level2_v1 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_level2_v1 --teams team01 --players 1 --width 12 --height 12
```
- **Average Level Achieved**: `1.8`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `1724.80`
- **Max Turns Survived**: `3909`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_142019_zappy_level2_v1_r2.md](../training/results/eval_20260627_142019_zappy_level2_v1_r2.md)*

#### Observations (when running game with gui)
2 game runs performed
- *Key observations*
  - Agent prioritizes taking food
  - The agent moves forward constantly but did perform some turns
  - Agent managed to get to level 2 both runs. It did not use the stones to evolve, but it did manage to evolve when the stones were already on the tile.
- *Did it exhibit any loop behaviors or stuck states?*
- *Adjustments needed for the next run:*
  - Change the excess food collection reward from +0.2 to 0.0. This forces it to ignore excess food and spend its energy exploring for stones to get the +4.0 stone reward.
  - We will continue training Phase 2. With the updated rewards
  - Make a new run loading `zappy_level2_v1` to generate `zappy_level2_v2`

---
updated evaluation script to get more info from it, and avoid having to run full game to see hawt the ai was doing

--- 

## Run #4
- **Base Model**: `Loaded from zappy_level2_v1`
- **Output Model Name**: `zappy_level2_v2`
- **Timesteps Run**: `500,000`
- **Real-World Duration**: `2m 52s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 500000 -f 1000 -m zappy_level2_v2 -l zappy_level2_v1 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_level2_v2 --teams team01 --players 1 --width 12 --height 12
```
- **Average Level Achieved**: `1.8`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `1065.40`
- **Max Turns Survived**: `3974`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_151904.md](../training/results/eval_20260627_151904.md)*

#### Observations
- *Key observations*
  - (Compared with the updated evaluation from v1: *[eval_20260627_145555_zappy_level2_v1_r2.md](../training/results/eval_20260627_145555_zappy_level2_v1_r2.md)*)
  - It stopped spamming blindly and only incants when conditions are correct.
  - The agent is actively dropping stones to prepare the ritual.
  - No more wasted actions. It stopped spamming the radio at Level 1.
  - Survival improved by over 180 turns.
  - It successfully reached Level 2 in 4 out of the 5 test episodes.
- *Adjustments needed for the next run:*
  - Continue training zappy_level2_v2 for another 300,000 timesteps to let it polish its speed and reach 100% success rate


## Run #5
- **Base Model**: `Loaded from zappy_level2_v2`
- **Output Model Name**: `zappy_level2_v2`
- **Timesteps Run**: `300,000`
- **Real-World Duration**: `1m 44s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 300000 -f 1000 -m zappy_level2_v2 -l zappy_level2_v2 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_level2_v1 --teams team01 --players 1 --width 12 --height 12
```
- **Average Level Achieved**: `1.80`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `3574.20`
- **Max Turns Survived**: `8230`
- **Rating Tier**: `Tier 2: Single-Player Competence`

Evaluation file: *[eval_20260627_153033.md](../training/results/eval_20260627_153033.md)*

#### Observations
- *Key observations*
  - Average turns survived increased by over 2,500 turns, indicating the agent is highly stable at finding food.
  - It only attempted 4 incantations and succeeded in 100% of them.
  - The shortcut. `Set Stone` was 0.0%. The agent realized that on a 12x12 single-player map, it is faster to wander until standing on a pre-spawned Linemate and incanting, rather than picking up and carrying stones.
  - Reached **Tier 2: Single-Player Competence**.
- *Adjustments needed for the next run:*
  - Transition to Phase 3. We will load the trained `zappy_level2_v2` model and train it under multi-agent conditions to force coordination, radio usage, and active stone-gathering (since multi-stone recipes cannot be solved by the shortcut).