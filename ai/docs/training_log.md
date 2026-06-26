# Zappy AI Training Log & Roadmap

This file acts as a lab notebook to document the parameter tweaks, learning curves, and results of training runs. Below is the proposed step-by-step training roadmap starting from scratch.

---

## AI Training Roadmap

```mermaid
graph TD
    Phase1[Phase 1: Basic Survival] -->|Learn to eat & navigate| Phase2[Phase 2: Single-Agent Elevation]
    Phase2 -->|Learn to collect stones & incant| Phase3[Phase 3: Coordination & Radio]
    Phase3 -->|Learn to fork, broadcast & gather teammates| Phase4[Phase 4: Full Scale Competitive]
```

### Phase 1: Basic Survival & Navigation
* **Objective**: Teach the neural network to walk, search the map, check its inventory, and consume food when hungry. It must learn not to starve.
* **Map Size**: `10x10`
* **Configuration**: `total_teams=1`, `frequency=1000`
* **Expected Result**: Survives average of 10,000+ turns, remains Level 1.
* **Timesteps**: `200,000`
* **Target Model Name**: `zappy_survival_v1`

### Phase 2: Single-Agent Elevation (Level 2 Competence)
* **Objective**: Train the agent to search for Linemate stones, drop them on a tile, and execute the `Incantation` command to elevate itself to Level 2.
* **Map Size**: `12x12`
* **Configuration**: Load `zappy_survival_v1`, `total_teams=1`, `frequency=1000`
* **Expected Result**: Reaches Level 2 consistently.
* **Timesteps**: `300,000` (Cumulative: `500,000`)
* **Target Model Name**: `zappy_level2_v1`

### Phase 3: Coordination & Radio Broadcasts (Level 3-4 Competence)
* **Objective**: Level 3 requires multiple players. Agents must learn to use `FORK` to spawn teammates, `BROADCAST` messages to coordinate, and navigate towards teammate coordinates when an incantation is initiated.
* **Map Size**: `15x15`
* **Configuration**: Load `zappy_level2_v1`, `total_teams=1` (with 2 players), `frequency=1000`
* **Expected Result**: Reaches Level 3/4.
* **Timesteps**: `500,000` (Cumulative: `1,000,000`)
* **Target Model Name**: `zappy_coordination_v1`

### Phase 4: Competitive Multi-Team Play (Level 5-8 Competence)
* **Objective**: Train in the presence of rival teams. Learn to manage resource scarcity, handle ejection, and optimize team-wide ascension to Level 8.
* **Map Size**: `20x20`
* **Configuration**: Load `zappy_coordination_v1`, `total_teams=2` (each with 2-3 players), `frequency=1000`
* **Expected Result**: Master coordination, reaching Level 5+ consistently.
* **Timesteps**: `1,000,000` (Cumulative: `2,000,000`)
* **Target Model Name**: `zappy_master_v1`

---

##  Experiment Log

template to document each training run.

### Run #[Number]
- **Base Model**: `[Fresh / Loaded from <model_name>]`
- **Output Model Name**: `[e.g. zappy_survival_v1]`
- **Timesteps Run**: `[e.g., 200,000]`
- **Real-World Duration**: `[e.g., 8 minutes]`

#### Parameters
```bash
# Paste the exact run command here:
./ai/training/run_training.sh -t 200000 -x 10 -y 10 -f 1000 -n TeamAI -m zappy_survival_v1
```

#### Evaluation Metrics (via evaluate_ai.py)
```
# Run: PYTHONPATH=ai python ai/training/training_env/evaluate_ai.py --model zappy_survival_v1
```
- **Average Level Achieved**: `X.XX`
- **Max Level Achieved**: `X`
- **Average Turns Survived**: `XXXX`
- **Rating Tier**: `[Tier 1 / Tier 2 / ...]`

#### Observations
- *What actions did the agent prioritize?*
- *Did it exhibit any loop behaviors or stuck states?*
- *Adjustments needed for the next run:*
