import sys
import os
import argparse
import pathlib
import numpy as np
import ctypes
from stable_baselines3 import PPO
from src.client.lib_client import ZappyLib, ZappyLibClient
from src.utils import Inventory
from training.training_env.actions import ControllerAction
from training.training_env.broadcast import BroadcastDict, BroadcastHandler
import datetime

# Resolve project path to allow imports of 'src' and 'training'
sys.path.insert(
    0,
    os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../..")),
)


def get_observation(client, verbose=False):
    """Generates the 657-dimensional observation vector for a client."""
    if client.is_dead:
        return np.zeros(657, dtype=np.int32)

    obs = np.zeros(657, dtype=np.int32)
    resources = [
        "player",
        "food",
        "linemate",
        "deraumere",
        "sibur",
        "mendiane",
        "phiras",
        "thystame",
    ]

    # 1. Parse inventory
    if verbose:
        print(f"    [Agent {client.player_id}] Sending Inventory...", flush=True)
    inv = client.inventory()
    if verbose:
        print(f"    [Agent {client.player_id}] Inventory response: {inv}", flush=True)
    if client.is_dead:
        return np.zeros(657, dtype=np.int32)

    if isinstance(inv, Inventory):
        obs[0] = inv.food
        obs[1] = inv.linemate
        obs[2] = getattr(inv, "deraumere", 0)
        obs[3] = getattr(inv, "sibur", 0)
        obs[4] = getattr(inv, "mendiane", 0)
        obs[5] = getattr(inv, "phiras", 0)
        obs[6] = getattr(inv, "thystame", 0)
        obs[656] = inv.food

    # 2. Parse vision
    if verbose:
        print(f"    [Agent {client.player_id}] Sending Look...", flush=True)
    vision_list = client.look()
    if verbose:
        print(
            f"    [Agent {client.player_id}] Look response: {vision_list}", flush=True
        )
    if client.is_dead:
        return np.zeros(657, dtype=np.int32)

    if isinstance(vision_list, list):
        base_index = 7
        for i, tile_str in enumerate(vision_list):
            if i >= 81:
                break
            if isinstance(tile_str, str):
                entities = tile_str.strip().split(" ")
            else:
                entities = tile_str
            for entity in entities:
                if entity in resources:
                    offset_resource = resources.index(entity)
                    obs[base_index + (i * 8) + offset_resource] += 1

    # 3. Current level
    obs[655] = client.level
    return obs


def perform_action(client, action_id, verbose=False):
    """Maps discrete action ID to ZappyLibClient FFI calls."""
    if client.is_dead:
        return "dead"

    try:
        bot_action = ControllerAction(int(action_id))
    except ValueError:
        return "ko"

    ZAPPY_ITEMS = [
        "food",
        "linemate",
        "deraumere",
        "sibur",
        "mendiane",
        "phiras",
        "thystame",
    ]

    if verbose:
        print(
            f"    [Agent {client.player_id}] Performing action {bot_action}...",
            flush=True,
        )
    res = "ko"
    if bot_action == ControllerAction.FORWARD:
        res = client.forward()
    elif bot_action == ControllerAction.LEFT:
        res = client.left()
    elif bot_action == ControllerAction.RIGHT:
        res = client.right()
    elif bot_action == ControllerAction.LOOK:
        res = client.look()
    elif bot_action == ControllerAction.INVENTORY:
        res = client.inventory()
    elif bot_action == ControllerAction.CONNECT_NBR:
        res = client.connect_nbr()
    elif bot_action == ControllerAction.FORK:
        res = client.fork()
    elif bot_action == ControllerAction.EJECT:
        res = client.eject()
    elif bot_action == ControllerAction.INCANTATION:
        res = client.incantation()
    elif bot_action == ControllerAction.BROADCAST:
        inv = client.inventory()
        if client.is_dead:
            return "dead"
        has_stones = (
            isinstance(inv, Inventory)
            and inv.linemate >= 1
            and inv.deraumere >= 1
            and inv.sibur >= 1
        )
        handler = BroadcastHandler(team_name=client.team_name, secret_key="ZAPPY_SEC")
        if client.level >= 2 and has_stones:
            msg = handler.build_message(BroadcastDict.INCANT)
        elif isinstance(inv, Inventory) and inv.food < 5:
            msg = handler.build_message(BroadcastDict.FIND, "food")
        else:
            msg = handler.build_message(BroadcastDict.FIND, "stones")
        res = client.broadcast(msg)
    elif ControllerAction.TAKE_FOOD <= bot_action <= ControllerAction.TAKE_THYSTAME:
        item = ZAPPY_ITEMS[bot_action - ControllerAction.TAKE_FOOD]
        res = client.take(item)
    elif ControllerAction.SET_FOOD <= bot_action <= ControllerAction.SET_THYSTAME:
        item = ZAPPY_ITEMS[bot_action - ControllerAction.SET_FOOD]
        res = client.set(item)
    if verbose:
        print(f"    [Agent {client.player_id}] Action response: {res}", flush=True)
    return res


def classify_performance(avg_level, avg_turns):
    if avg_level < 2 and avg_turns < 2000:
        return " Tier 1: Starvation/Survival Failure"
    elif avg_level < 3:
        return " Tier 2: Single-Player Competence"
    elif avg_level < 5:
        return " Tier 3: Early Coordination (Level 3-4)"
    elif avg_level < 7:
        return " Tier 4: Advanced Teamwork (Level 5-6)"
    elif avg_level < 8:
        return " Tier 5: Master Ascension (Level 7)"
    else:
        return " Tier 6: Celestial Zenith (Level 8 Victory)"


def main():
    parser = argparse.ArgumentParser(description="Evaluate Zappy AI Model Head-lessly")
    parser.add_argument(
        "--model", type=str, default="zappy_ai_model", help="Saved model path/name"
    )
    parser.add_argument(
        "--episodes", type=int, default=5, help="Number of test episodes to run"
    )
    parser.add_argument(
        "--freq", type=int, default=1000, help="Simulation tick frequency"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print detailed step-by-step logs of agent actions",
    )
    parser.add_argument("--width", type=int, default=15, help="Map width")
    parser.add_argument("--height", type=int, default=15, help="Map height")
    parser.add_argument(
        "--teams",
        type=str,
        default="team01,team02",
        help="Comma-separated list of team names",
    )
    parser.add_argument(
        "--players", type=int, default=2, help="Number of players per team"
    )
    args = parser.parse_args()

    # Locate model path
    model_name = args.model
    model_path = model_name
    possible_paths = [
        pathlib.Path(model_name),
        pathlib.Path(__file__).resolve().parents[3] / model_name,
        pathlib.Path(__file__).resolve().parents[2] / model_name,
    ]
    for p in possible_paths:
        if p.with_suffix(".zip").is_file() or p.is_file():
            model_path = str(p)
            break

    print(f"[Eval] Loading model from {model_path}...")
    try:
        model = PPO.load(model_path)
    except Exception as e:
        print(f"[Error] Failed to load model: {e}")
        sys.exit(84)

    # Initialize headless library FFI
    print("[Eval] Initializing ZappyLib direct engine...")
    try:
        zappy_lib = ZappyLib()
    except Exception as e:
        print(
            f"[Error] Failed to load direct library: {e}. Please build the server target library first."
        )
        sys.exit(84)

    width = args.width
    height = args.height
    teams = [t.strip() for t in args.teams.split(",")]
    num_teams = len(teams)
    clients_nb = args.players

    all_turns = []
    all_levels = []

    print(
        f"[Eval] Starting evaluation: {args.episodes} episodes ({num_teams} teams of {clients_nb} players each)..."
    )

    for ep in range(args.episodes):
        # 1. Initialize server
        TeamArray = ctypes.c_char_p * num_teams
        team_ptrs = TeamArray(*[t.encode("utf-8") for t in teams])
        server_ptr = zappy_lib.lib.zappy_init(
            width, height, args.freq, team_ptrs, num_teams, clients_nb
        )

        # 2. Add players and clients dynamically
        clients = []
        for team_name in teams:
            for _ in range(clients_nb):
                p_id = zappy_lib.lib.zappy_add_player(
                    server_ptr, team_name.encode("utf-8")
                )
                client = ZappyLibClient(zappy_lib.lib, server_ptr, p_id, args.freq)
                client.team_name = team_name
                clients.append(client)

        # Consume initial server logs
        for c in clients:
            for _ in range(2):
                c.wait_for_response()

        turns = 0
        max_turns = 10000  # Safety cap to avoid infinite loops

        # Episode Loop
        while turns < max_turns:
            active_clients = [c for c in clients if not c.is_dead]
            if not active_clients:
                break

            if turns % 100 == 0:
                levels_str = ", ".join(
                    f"C{i + 1}:L{c.level}" for i, c in enumerate(clients)
                )
                print(
                    f"  [Episode {ep + 1}/{args.episodes}] Turn {turns} | {levels_str} | Active: {len(active_clients)}   ",
                    end="\r",
                    flush=True,
                )

            for client in active_clients:
                try:
                    obs = get_observation(client, verbose=args.verbose)
                    if client.is_dead:
                        continue
                    action, _ = model.predict(obs, deterministic=True)
                    perform_action(client, action, verbose=args.verbose)
                except Exception:
                    client.is_dead = True

            turns += 1

        # Collect metrics
        ep_levels = [c.level for c in clients]
        all_levels.extend(ep_levels)
        all_turns.append(turns)

        zappy_lib.lib.zappy_free(server_ptr)
        print(
            f"  Episode {ep + 1}/{args.episodes} finished. Max Level: {max(ep_levels)} | Turns survived: {turns}"
        )

    # Print summary statistics
    avg_level = float(np.mean(all_levels))
    max_level = int(np.max(all_levels))
    avg_turns = float(np.mean(all_turns))
    max_turns_seen = int(np.max(all_turns))
    tier = classify_performance(avg_level, avg_turns)

    # 1. Print to console
    print("\n" + "=" * 50)
    print("           EVALUATION REPORT")
    print("=" * 50)
    print("| Metric                 | Average      | Max         |")
    print("|------------------------|--------------|-------------|")
    print(f"| Level Achieved         | {avg_level:<12.2f} | {max_level:<11d} |")
    print(f"| Turns Survived         | {avg_turns:<12.2f} | {max_turns_seen:<11d} |")
    print("-" * 50)
    print(f" RATING TIER: {tier}")
    print("=" * 50 + "\n")

    # 2. Save results to files
    results_dir = pathlib.Path(__file__).resolve().parents[1] / "results"
    results_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    history_file = results_dir / f"eval_{timestamp}.md"
    latest_file = results_dir / "latest_eval.md"

    report_content = f"""# Strict Evaluation Report

**Date**: {datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
**Model**: `{model_name}`
**Episodes**: {args.episodes}
**Map Size**: `{width}x{height}`
**Teams**: `{", ".join(teams)} ({clients_nb} players/team)`

| Metric | Average | Max |
| :--- | :---: | :---: |
| Level Achieved | {avg_level:.2f} | {max_level} |
| Turns Survived | {avg_turns:.2f} | {max_turns_seen} |

**Rating Tier**: {tier}
"""

    try:
        with open(history_file, "w") as f:
            f.write(report_content)
        with open(latest_file, "w") as f:
            f.write(report_content)
        print("[Eval] Results successfully saved to:")
        print(f"  - {history_file}")
        print(f"  - {latest_file}\n")
    except Exception as e:
        print(f"[Warning] Failed to save evaluation files: {e}\n")


if __name__ == "__main__":
    main()
