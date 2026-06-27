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
- **Rating Tier**: `Tier 2`

Evaluation file: *[eval_20260627_153033.md](../training/results/eval_20260627_153033.md)*

#### Observations
- *Key observations*
  - Average turns survived increased by over 2,500 turns, indicating the agent is highly stable at finding food.
  - It only attempted 4 incantations and succeeded in 100% of them.
  - The shortcut. `Set Stone` was 0.0%. The agent realized that on a 12x12 single-player map, it is faster to wander until standing on a pre-spawned Linemate and incanting, rather than picking up and carrying stones.
  - Reached **Tier 2: Single-Player Competence**.
- *Adjustments needed for the next run:*
  - Transition to Phase 3. We will load the trained `zappy_level2_v2` model and train it under multi-agent conditions to force coordination, radio usage, and active stone-gathering (since multi-stone recipes cannot be solved by the shortcut).


## Run #6
- **Base Model**: `Loaded from zappy_level2_v2`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `500,000`
- **Real-World Duration**: `2m 55s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 500000 -f 1000 -m zappy_coordination_v1 -l zappy_level2_v2 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.10`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `74.80`
- **Max Turns Survived**: `89`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_154847.md](../training/results/eval_20260627_154847.md)*

#### Observations
- *Key observations*
  - Because the evaluation script runs clients sequentially and ticks the server for each command, time passed twice as fast relative to the actions they took. This caused both players to consume food twice as fast and starve in ~74 turns.
  - During training, the second teammate is a dummy player that stands completely still. It never eats, never collects stones, and stays at Level 1 forever. Therefore, it was mathematically impossible for the learning agent to ever successfully elevate to Level 3 during training.
- *Adjustments needed for the next run:*
  - To train Phase 3 successfully, the teammate cannot be a dummy. It must be active, survive, reach Level 2, and coordinate. 
  - We can achieve this by using the existing heuristic logic (`take_decision` from *src.strategy.decision_making*) to run the teammate players in the background during training.


## Run #7
- **Base Model**: `Loaded from zappy_level2_v2`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `500,000`
- **Real-World Duration**: `3m 28s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 500000 -f 1000 -m zappy_coordination_v1 -l zappy_level2_v2 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.40`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `84.80`
- **Max Turns Survived**: `165`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_161540.md](../training/results/eval_20260627_161540.md)*

#### Observations
- *Key observations*
  - Because you are evaluating with 2 active players, they tick the server twice as fast, meaning they starve in fewer rounds. Now that they train with active teammate bots, they are experiencing this faster food consumption during training and are beginning to learn how to deal with it.
- *Adjustments needed for the next run:*
  - Need to give the model more training time to optimize its pathing, food collection, and coordinate meeting speed.


## Run #8
- **Base Model**: `Loaded from zappy_coordination_v1`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `1,000,000`
- **Real-World Duration**: `6m 37s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 1000000 -f 1000 -m zappy_coordination_v1 -l zappy_coordination_v1 
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.40`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `57.00`
- **Max Turns Survived**: `71`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_164313.md](../training/results/eval_20260627_164313.md)*

#### Observations
- *Key observations*
  - The agent spent 55.8% of its actions on Take Food and 25.4% on Vision & Info. 
  - Movement fell to 17.2% and Broadcast dropped to 0.0%. 
  - Turns Survived dropped from 84 to 57.
  -  The time-acceleration starvation pressure (caused by both players ticking the server sequentially) was simply too strong. Over 1,000,000 steps of training, PPO got stuck in a local minimum: "Moving and broadcasting is too expensive and leads to quick starvation. I must stand in one place, check my inventory, and take food. "By refusing to move, they ate all the food on their spawn tile and starved even faster (hence the drop to 57 turns).
- *Adjustments needed for the next run:*
  - Update `ZappyEnv.py` to rate-limit the teammate bots to run only once every 5 steps. 
  - In 4 out of 5 turns, only the PPO agent is active and ticking the server. This reduces the tick-acceleration by 80%, returning food consumption to almost normal (1x). 
  - Run a fresh training run for Phase 3 for 500,000 timesteps loading from your stable `zappy_level2_v2` model


## Run #9
- **Base Model**: `Loaded from zappy_level2_v2`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `500,000`
- **Real-World Duration**: `2m 57s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 500000 -f 1000 -m zappy_coordination_v1 -l zappy_level2_v2
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.80`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `49.40`
- **Max Turns Survived**: `75`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_170407.md](../training/results/eval_20260627_170407.md)*

#### Observations
- *Key observations*
  - Unlike Run #8, the policy did not collapse. The agent spent 49.2% of its actions moving and only 21.8% taking food, demonstrating a healthy balance of survival and exploration.
  - Reached an average level of 1.80 (with a max of 2.0). They achieved 8 successful incantations out of 15 attempts (53.3% success rate).
  - Broadcasts were 0% because players spent their short lives at Level 1 collecting Linemates. Since Level 1 -> Level 2 is a single-player incantation, they did not need to broadcast.
  - Average turns survived was 49.4 turns because the evaluation script still runs sequential ticking (2x starvation). However, the training environment rate-limiting worked perfectly, allowing the model to train successfully.
- *Adjustments needed for the next run:*
  - Fix ticking distortion in the evaluation script.
  - Longer training run of 2,000,000 timesteps loading from `zappy_coordination_v1` so the agent has enough time to practice and optimize its Level 2 stone-gathering and broadcast behaviors.


## Run #10
- **Base Model**: `Loaded from zappy_coordination_v1`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `11m 56s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v1 -l zappy_coordination_v1
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.20`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `81.60`
- **Max Turns Survived**: `85`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_173747.md](../training/results/eval_20260627_173747.md)*

#### Observations
- *Key observations*
  - The new batching logic in `evaluate_ai.py` worked successfully. The server clock now ticks normally (1x starvation rate), extending player survival turns to 81.6 turns (up from 49.4 turns).
  - Reaching Level 2 was achieved in some cases, but average level achieved dropped to 1.20 and `Set Stone` surged to 76.3%. 
  - This is a direct result of the clock fix. In Run #9 (2x starvation), the players died almost immediately after reaching Level 2, hiding their behavior. Now that they live longer (81.6 turns), they spend 50+ turns stuck at Level 2. Because they only trained for 500,000 steps, they have not learned how to coordinate or gather Deraumere/Sibur, so they resort to spamming `Set Stone` (their fallback policy).
- *Adjustments needed for the next run:*
  - Train the model for another 2,000,000 timesteps loading from the current `zappy_coordination_v1`. This will give the agent the necessary iterations at Level 2 to learn how to gather Deraumere/Sibur and use the radio to broadcast.


## Run #11
- **Base Model**: `Loaded from zappy_coordination_v1`
- **Output Model Name**: `zappy_coordination_v1`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `11m 41s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v1 -l zappy_coordination_v1
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.50`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `80.00`
- **Max Turns Survived**: `144`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_181943.md](../training/results/eval_20260627_181943.md)*

#### Observations
- *Key observations*
  - `Set Stone` dropped to **0.0%**. The spamming fallback bug is completely resolved.
  - `Vision & Info` (Look/Inventory) jumped to **65.9%**. Because the agent has no coordination route to Level 3, it farms safe actions that yield `0.0` points to avoid the negative penalties of movement/starvation and failed actions (`-0.1` to `-10.0`).
  - Discovered that the standard teammate bot broadcasts in standard text format (`Incantation TeammateBot_X level 2`), while the PPO agent's `BroadcastHandler` only deciphers pipe-separated formats (`team1|ZAPPY_SEC|...`). As a result, the PPO agent was completely deaf to teammate coordinates during the entire training.
- *Adjustments needed for the next run:*
  - Modified `lib_client.py` to intercept and translate teammate broadcasts to `team1|ZAPPY_SEC|INCANT` or `COME`.
  - Modified `ZappyEnv.py` so Level 2 teammates periodically broadcast a `COME` beacon every 10 steps.
  - Execute a 2,000,000 step training run loading from `zappy_coordination_v1` so the agent can learn to follow the newly unlocked radio signal and coordinate to reach Level 3.


## Run #12
- **Base Model**: `Loaded from zappy_coordination_v1`
- **Output Model Name**: `zappy_coordination_v1_2`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `11m 58s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v1_2 -l zappy_coordination_v1
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1_2 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.50`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `105.40`
- **Max Turns Survived**: `159`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_184331.md](../training/results/eval_20260627_184331.md)*

#### Observations
- *Key observations*
  - Average turns survived increased to 105.40 turns (Max 159).
  - `Vision & Info` count jumped to 90.2% (943 times) and `Movement` dropped to 6.7% (70 times).
  - Discovered that the PPO agent's 657-element observation vector did **not** contain any features representing the teammate's broadcast direction. The agent was mathematically blind to the radio signal, explaining why it got stuck spamming `LOOK` (which yields 0.0 points) to minimize loss while waiting to die.
- *Adjustments needed for the next run:*
  - Overwrote the redundant food-copy feature at `obs[656]` to contain the coordinate direction (`best_heuristic["dir"]`) of the teammate's broadcast.
  - **Train Run #13**: Run a 2,000,000 step training run loading from `zappy_coordination_v1_2` so the agent can learn to map this newly visible broadcast direction input to correct steering actions and reach Level 3!


## Run #13
- **Base Model**: `Loaded from zappy_coordination_v1_2`
- **Output Model Name**: `zappy_coordination_v1_2`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `12m`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v1_2 -l zappy_coordination_v1_2
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v1_2 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.70`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `90.80`
- **Max Turns Survived**: `159`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_200328.md](../training/results/eval_20260627_200328.md)*

#### Observations
- *Key observations*
  - Reached an average level of 1.70 (7 out of 10 players successfully reached Level 2).
  - `Vision & Info` remained very high at 85.9%, and `Movement` was low at **9.6%**. 
  - Although we added the broadcast coordinate direction input at `obs[656]` and activated the beacons, the model remained stuck in the `LOOK`/`INVENTORY` spamming loop. This is because it was trained by loading from a collapsed base model (`zappy_coordination_v1_2` from Run #12) whose neural network weights were already heavily saturated with the passive farming bias. Breaking out of such a local minimum in PPO is extremely difficult due to low policy entropy.
- *Adjustments needed for the next run:*
  - Clean Restart. Run a fresh 2,000,000 timestep training run loading from the clean, stable **`zappy_level2_v2`** model. Since `zappy_level2_v2` has no prior bias towards spamming `LOOK` and knows how to survive and gather, it will immediately learn to map the active beacon coordinate inputs to correct steering movements and reach Level 3


## Run #14
- **Base Model**: `Loaded from zappy_coordination_v1_2`
- **Output Model Name**: `zappy_coordination_v2`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `12m`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v2 -l zappy_level2_v2
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v2 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.60`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `64.60`
- **Max Turns Survived**: `85`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_205408.md](../training/results/eval_20260627_205408.md)*

#### Observations
- *Key observations*
  - Verified that the new level-up tracking works perfectly, showing 6 successful incantations out of 27 attempts (representing players successfully leveling up from Level 1 to Level 2).
  - `Take Food` was 92.5% and `Movement` was only 3.3%.
  -  When the teammate bot broadcasts a beacon, the PPO agent was getting penalized `-0.5` points on any action that wasn't moving towards the teammate (including necessary survival actions like `TAKE_FOOD`, `LOOK`, or `INVENTORY`). As a result, the agent learned that moving at all was too risky and collapsed into a static food-hoarding policy to minimize starvation losses.
- *Adjustments needed for the next run:*
  - Modified `ZappyEnv.py` to only apply the `-0.5` ignore penalty if the agent chose an incorrect movement action (`FORWARD`, `LEFT`, `RIGHT`) in the wrong direction, rather than penalizing it for survival tasks.
  - Start a fresh 2,000,000 timestep training run loading from `zappy_level2_v2` with the refined reward structure. This will keep the agent's natural survival behaviors intact while safely steering it toward the teammate beacon!


## Run #15
- **Base Model**: `Loaded from zappy_level2_v2`
- **Output Model Name**: `zappy_coordination_v2`
- **Timesteps Run**: `2,000,000`
- **Real-World Duration**: `11m 52s`

#### Parameters
```bash
# Paste the exact run command here:
./run_training.sh -t 2000000 -f 1000 -m zappy_coordination_v2 -l zappy_level2_v2
```

#### Evaluation Metrics (via evaluate_ai.py)
Run (adjust --teams, --players, --width, --height depending on phase):
```
PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_coordination_v2 --teams team01 --player 2 --width 15 --height 15
```
- **Average Level Achieved**: `1.80`
- **Max Level Achieved**: `2`
- **Average Turns Survived**: `53.00`
- **Max Turns Survived**: `81`
- **Rating Tier**: `Tier 1`

Evaluation file: *[eval_20260627_220735.md](../training/results/eval_20260627_220735.md)*

#### Observations
- *Key observations*
  - Reached an average level of 1.80 (8 players successfully elevated to Level 2).
  - `Take Food` was still very high at 85.0% and `Movement` was only 7.5%
  - Picking up food yielded a massive `+2.0` reward (when inventory was < 15), while moving forward only gave `+0.1` and consumed energy. The agent learned that standing still and waiting for food to spawn on its tile to pick it up was a low-risk, highly lucrative reward farming loop, completely overshadowing navigation and stone gathering.
- *Adjustments needed for the next run:*
  - Reduced `Take Food` reward in `ZappyEnv.py` from `2.0` to `0.2` (making it enough to incentivize survival but too small to farm). This ensures coordinate beacon steering (`+3.0`) and stone collection (`+4.0`) are the dominant reward sources.
  - Run a fresh 2,000,000 timestep training run loading from `zappy_level2_v2` with the reduced food reward.
