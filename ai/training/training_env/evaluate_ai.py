import sys
import os
import argparse
import pathlib
import numpy as np
import ctypes
from stable_baselines3 import PPO
from src.client.lib_client import ZappyLib, ZappyLibClient
from src.utils import Inventory, parse_look
from training.training_env.actions import ControllerAction
from training.training_env.broadcast import BroadcastDict, BroadcastHandler
import datetime
import re

# Resolve project path to allow imports of 'src' and 'training'
sys.path.insert(
    0,
    os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../..")),
)


def parse_inventory_resp(resp):
    if resp and resp.startswith("["):
        matches = re.findall(r"([a-zA-Z]+)[^0-9a-zA-Z]*(\d+)", resp)
        clean_resp = "[" + ", ".join(f"{name} {count}" for name, count in matches) + "]"
        return Inventory.from_string(clean_resp)
    return resp


def build_observation(client, inv, look_resp, verbose=False):
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

    parsed_inv = parse_inventory_resp(inv)
    # Cache inventory for broadcasts in action selection
    client.cached_inventory = parsed_inv

    if isinstance(parsed_inv, Inventory):
        obs[0] = parsed_inv.food
        obs[1] = parsed_inv.linemate
        obs[2] = getattr(parsed_inv, "deraumere", 0)
        obs[3] = getattr(parsed_inv, "sibur", 0)
        obs[4] = getattr(parsed_inv, "mendiane", 0)
        obs[5] = getattr(parsed_inv, "phiras", 0)
        obs[6] = getattr(parsed_inv, "thystame", 0)
        obs[656] = parsed_inv.food

    vision_list = (
        parse_look(look_resp) if look_resp and look_resp.startswith("[") else look_resp
    )
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

    obs[655] = client.level
    obs[656] = parsed_inv.food if isinstance(parsed_inv, Inventory) else 10
    obs[79] = 0
    return obs


def get_action_command_bytes(client, action_id):
    try:
        bot_action = ControllerAction(int(action_id))
    except ValueError:
        return b""

    ZAPPY_ITEMS = [
        "food",
        "linemate",
        "deraumere",
        "sibur",
        "mendiane",
        "phiras",
        "thystame",
    ]

    if bot_action == ControllerAction.FORWARD:
        return b"Forward\n"
    elif bot_action == ControllerAction.LEFT:
        return b"Left\n"
    elif bot_action == ControllerAction.RIGHT:
        return b"Right\n"
    elif bot_action == ControllerAction.LOOK:
        return b"Look\n"
    elif bot_action == ControllerAction.INVENTORY:
        return b"Inventory\n"
    elif bot_action == ControllerAction.CONNECT_NBR:
        return b"Connect_nbr\n"
    elif bot_action == ControllerAction.FORK:
        return b"Fork\n"
    elif bot_action == ControllerAction.EJECT:
        return b"Eject\n"
    elif bot_action == ControllerAction.INCANTATION:
        return b"Incantation\n"
    elif bot_action == ControllerAction.BROADCAST:
        inv = getattr(client, "cached_inventory", None)
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
        return f"Broadcast {msg}\n".encode("utf-8")
    elif ControllerAction.TAKE_FOOD <= bot_action <= ControllerAction.TAKE_THYSTAME:
        item = ZAPPY_ITEMS[bot_action - ControllerAction.TAKE_FOOD]
        return f"Take {item}\n".encode("utf-8")
    elif ControllerAction.SET_FOOD <= bot_action <= ControllerAction.SET_THYSTAME:
        item = ZAPPY_ITEMS[bot_action - ControllerAction.SET_FOOD]
        return f"Set {item}\n".encode("utf-8")

    return b""


def wait_for_response_no_tick(client):
    resp_ptr = client.lib.zappy_get_response(client.server_ptr, client.player_id)
    if resp_ptr:
        resp = ctypes.cast(resp_ptr, ctypes.c_char_p).value.decode("utf-8").strip()
        client.lib.zappy_free_string(resp_ptr)

        if resp == "dead":
            client.is_dead = True
            return "dead"

        if resp.startswith("Current level:"):
            try:
                client.level = int(resp.split(":")[1].strip())
            except Exception:
                pass
        return resp
    return None


def batch_send_command(clients, command_bytes_list, max_wait_ticks=2000):
    if not clients:
        return {}

    for client, cmd in zip(clients, command_bytes_list):
        if not client.is_dead and cmd:
            client.lib.zappy_send_command(client.server_ptr, client.player_id, cmd)

    ticks = 0
    responses = {client: None for client in clients if not client.is_dead}
    tick_size_ms = max(1, int(1000 / clients[0].freq))

    while ticks < max_wait_ticks:
        all_done = True
        for client in responses:
            if responses[client] is None:
                resp = wait_for_response_no_tick(client)
                if resp is not None:
                    cmd = command_bytes_list[clients.index(client)]
                    cmd_str = (
                        cmd.decode("utf-8").strip() if isinstance(cmd, bytes) else ""
                    )

                    is_async = False
                    if (
                        resp.startswith("message")
                        or resp.startswith("eject:")
                        or resp.startswith("event_start")
                        or resp.startswith("event_end")
                        or resp == "elevation en cours"
                    ):
                        is_async = True
                    elif cmd_str in ["Inventory", "Look"]:
                        if not resp.startswith("[") and resp != "dead":
                            is_async = True
                    else:
                        if resp not in ["ok", "ko", "dead"] and not resp.startswith(
                            "Current level:"
                        ):
                            is_async = True

                    if is_async:
                        if resp.startswith("message"):
                            from src.client.lib_client import parse_broadcast

                            parsed = parse_broadcast(resp)
                            if parsed:
                                client.messages.append(parsed)
                        all_done = False
                    else:
                        responses[client] = resp
                else:
                    all_done = False

        if all_done:
            break

        clients[0].lib.zappy_tick(clients[0].server_ptr, tick_size_ms)
        ticks += 1

    for client in list(responses.keys()):
        if responses[client] is None:
            client.is_dead = True
            responses[client] = "dead"

    return responses


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
        "--episodes", type=int, default=50, help="Number of test episodes to run"
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
    parser.add_argument(
        "--teammates",
        action="store_true",
        help="Use rule-based teammate bots for the other players on the team (matching training env)",
    )
    args = parser.parse_args()

    # Locate model path
    model_name = args.model
    model_path = model_name
    possible_paths = [
        pathlib.Path(model_name),
        pathlib.Path(__file__).resolve().parents[1] / "models" / model_name,
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

    stats = {
        "actions": {a.name: 0 for a in ControllerAction},
        "incantations_attempted": 0,
        "incantations_succeeded": 0,
        "items_taken": {},
        "items_set": {},
        "deaths": 0,
    }

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
        ppo_clients = []
        teammate_clients = []
        for team_name in teams:
            for i in range(clients_nb):
                p_id = zappy_lib.lib.zappy_add_player(
                    server_ptr, team_name.encode("utf-8")
                )
                client = ZappyLibClient(zappy_lib.lib, server_ptr, p_id, args.freq)
                client.team_name = team_name
                if args.teammates and i > 0:
                    client.name = f"TeammateBot_{p_id}"
                    teammate_clients.append(client)
                else:
                    client.name = f"Player_{p_id}"
                    ppo_clients.append(client)
                clients.append(client)

        # Consume initial server logs
        for c in clients:
            for _ in range(2):
                c.wait_for_response()

        turns = 0
        max_turns = 10000  # Safety cap to avoid infinite loops
        dead_clients = set()

        # Episode Loop
        while turns < max_turns:
            # Sync dead clients to stats
            for client in clients:
                if client.is_dead and client not in dead_clients:
                    dead_clients.add(client)
                    if client in ppo_clients:
                        stats["deaths"] += 1

            active_clients = [c for c in clients if not c.is_dead]
            if not active_clients:
                break

            previous_levels = {c: c.level for c in active_clients}

            if turns % 100 == 0:
                levels_str = ", ".join(
                    f"C{i + 1}:L{c.level}" for i, c in enumerate(clients)
                )
                print(
                    f"  [Episode {ep + 1}/{args.episodes}] Turn {turns} | {levels_str} | Active: {len(active_clients)}   ",
                    end="\r",
                    flush=True,
                )

            # 1. Batch query Inventory for all active clients
            inventories = batch_send_command(
                active_clients, [b"Inventory\n"] * len(active_clients)
            )

            # 2. Batch query Look for all active clients
            looks = batch_send_command(
                active_clients, [b"Look\n"] * len(active_clients)
            )

            # Mark dead clients
            for client in active_clients:
                inv = inventories.get(client)
                look_resp = looks.get(client)
                if inv == "dead" or look_resp == "dead" or client.is_dead:
                    client.is_dead = True

            # 2.5 Parse broadcasts for all active clients to find coordinate direction
            for client in active_clients:
                unread = client.get_unread_messages()
                best_h = {"score": 0, "dir": 0, "text": ""}
                handler = BroadcastHandler(
                    team_name=client.team_name, secret_key="ZAPPY_SEC"
                )
                for msg in unread:
                    direction = msg.get("direction", 0)
                    text = msg.get("text", "")
                    h = handler.calculate_heuristic(direction, text)
                    if h["score"] > best_h["score"]:
                        best_h = h
                        best_h["dir"] = direction
                        best_h["text"] = text
                client.last_broadcast_dir = best_h["dir"]
                client.last_broadcast_text = best_h["text"]

            # 3. Build observations and predict actions for PPO agents only
            actions = {}
            active_ppo = [c for c in ppo_clients if not c.is_dead]
            for client in active_ppo:
                inv = inventories.get(client)
                look_resp = looks.get(client)

                if inv == "dead" or look_resp == "dead" or client.is_dead:
                    client.is_dead = True
                    if client not in dead_clients:
                        dead_clients.add(client)
                        stats["deaths"] += 1
                    continue

                # Parse inventory for hybrid logic
                parsed_inv = parse_inventory_resp(inv)
                best_dir = getattr(client, "last_broadcast_dir", 0)
                best_text = getattr(client, "last_broadcast_text", "")

                vision_list = (
                    parse_look(look_resp)
                    if look_resp and look_resp.startswith("[")
                    else look_resp
                )

                obs = build_observation(client, inv, look_resp, verbose=args.verbose)

                from src.strategy.state_machine import get_hybrid_action

                action, _ = get_hybrid_action(
                    client=client,
                    parsed_inv=parsed_inv,
                    vision_list=vision_list,
                    current_level=client.level,
                    best_dir=best_dir,
                    best_text=best_text,
                    model=model,
                    obs=obs,
                )

                actions[client] = action
                if args.verbose:
                    food_count = (
                        parsed_inv.food if isinstance(parsed_inv, Inventory) else 10
                    )
                    print(
                        f"[Turn {turns}] Client {client.player_id}: Food={food_count}, RawInv={repr(inv)}, Action={ControllerAction(action).name}, Path={getattr(client, 'path_steps', [])}"
                    )

            # 4. Batch execute the actions
            clients_to_act = [c for c in active_clients if c in actions]
            if clients_to_act:
                action_bytes_list = [
                    get_action_command_bytes(c, actions[c]) for c in clients_to_act
                ]
                action_responses = batch_send_command(clients_to_act, action_bytes_list)

                for client in clients_to_act:
                    res = action_responses.get(client)
                    action = actions[client]
                    bot_action = ControllerAction(int(action))
                    stats["actions"][bot_action.name] += 1

                    if bot_action == ControllerAction.INCANTATION:
                        stats["incantations_attempted"] += 1

                    if bot_action == ControllerAction.TAKE_FOOD:
                        if res == "ok":
                            stats["items_taken"]["food"] = (
                                stats["items_taken"].get("food", 0) + 1
                            )
                    elif (
                        ControllerAction.TAKE_LINEMATE
                        <= bot_action
                        <= ControllerAction.TAKE_THYSTAME
                    ):
                        if res == "ok":
                            item_name = bot_action.name.replace("TAKE_", "").lower()
                            stats["items_taken"][item_name] = (
                                stats["items_taken"].get(item_name, 0) + 1
                            )
                    elif bot_action == ControllerAction.SET_FOOD:
                        if res == "ok":
                            stats["items_set"]["food"] = (
                                stats["items_set"].get("food", 0) + 1
                            )
                    elif (
                        ControllerAction.SET_LINEMATE
                        <= bot_action
                        <= ControllerAction.SET_THYSTAME
                    ):
                        if res == "ok":
                            item_name = bot_action.name.replace("SET_", "").lower()
                            stats["items_set"][item_name] = (
                                stats["items_set"].get(item_name, 0) + 1
                            )

                    if res == "dead" or client.is_dead:
                        client.is_dead = True
                        if client in ppo_clients and client not in dead_clients:
                            dead_clients.add(client)
                            stats["deaths"] += 1

            # Check for successful level ups
            for client in active_ppo:
                prev_lvl = previous_levels.get(client, 1)
                if client.level > prev_lvl:
                    stats["incantations_succeeded"] += client.level - prev_lvl

            # Run teammate bot decisions (once every 5 turns)
            if args.teammates and teammate_clients and (turns % 5 == 0):
                from src.strategy.decision_making import take_decision

                for teammate in teammate_clients:
                    if not teammate.is_dead:
                        try:
                            if teammate.level >= 2 and (turns % 10 == 0):
                                teammate.broadcast("COME")
                            take_decision(teammate)
                        except Exception:
                            teammate.is_dead = True

            turns += 1

        # Collect metrics
        ep_levels = [c.level for c in ppo_clients]
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

    # Calculate detailed action analytics
    total_actions = sum(stats["actions"].values())

    movement_actions = ["FORWARD", "LEFT", "RIGHT"]
    vision_actions = ["LOOK", "INVENTORY", "CONNECT_NBR"]
    take_stone_actions = [
        a.name
        for a in ControllerAction
        if a.name.startswith("TAKE_") and a.name != "TAKE_FOOD"
    ]
    set_stone_actions = [
        a.name
        for a in ControllerAction
        if a.name.startswith("SET_") and a.name != "SET_FOOD"
    ]

    movement_count = sum(stats["actions"].get(a, 0) for a in movement_actions)
    vision_count = sum(stats["actions"].get(a, 0) for a in vision_actions)
    take_food_count = stats["actions"].get("TAKE_FOOD", 0)
    take_stone_count = sum(stats["actions"].get(a, 0) for a in take_stone_actions)
    set_stone_count = sum(stats["actions"].get(a, 0) for a in set_stone_actions)
    incant_count = stats["actions"].get("INCANTATION", 0)
    broadcast_count = stats["actions"].get("BROADCAST", 0)
    fork_count = stats["actions"].get("FORK", 0)

    def pct(count):
        return (count / total_actions * 100) if total_actions > 0 else 0.0

    # Print to console
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

    print("=" * 50)
    print("           BEHAVIOR ANALYSIS")
    print("=" * 50)
    print("| Event / Action Category| Count        | % of Total  |")
    print("|------------------------|--------------|-------------|")
    print(
        f"| Movement (Fwd/L/R)     | {movement_count:<12d} | {pct(movement_count):<10.1f}% |"
    )
    print(
        f"| Vision & Info (Look/Iv)| {vision_count:<12d} | {pct(vision_count):<10.1f}% |"
    )
    print(
        f"| Take Food              | {take_food_count:<12d} | {pct(take_food_count):<10.1f}% |"
    )
    print(
        f"| Take Stone             | {take_stone_count:<12d} | {pct(take_stone_count):<10.1f}% |"
    )
    print(
        f"| Set Stone              | {set_stone_count:<12d} | {pct(set_stone_count):<10.1f}% |"
    )
    print(
        f"| Broadcast Radio        | {broadcast_count:<12d} | {pct(broadcast_count):<10.1f}% |"
    )
    print(f"| Fork Allied Slots      | {fork_count:<12d} | {pct(fork_count):<10.1f}% |")
    print(
        f"| Incantations Attempted | {incant_count:<12d} | {pct(incant_count):<10.1f}% |"
    )
    print("-" * 50)
    print(
        f"| Incantations Succeeded | {stats['incantations_succeeded']:<12d} | (Succeed rate: {(stats['incantations_succeeded'] / stats['incantations_attempted'] * 100) if stats['incantations_attempted'] > 0 else 0:.1f}%) |"
    )
    print(f"| Starved Deaths         | {stats['deaths']:<12d} |")
    print("=" * 50 + "\n")

    # Save results to files
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

## Overall Performance Metrics
| Metric | Average | Max |
| :--- | :---: | :---: |
| Level Achieved | {avg_level:.2f} | {max_level} |
| Turns Survived | {avg_turns:.2f} | {max_turns_seen} |

**Rating Tier**: {tier}

## Agent Behavior Distribution
| Event / Action Category | Count | % of Total |
| :--- | :---: | :---: |
| Movement (Forward, Left, Right) | {movement_count} | {pct(movement_count):.1f}% |
| Vision & Info (Look, Inventory, Connect_nbr) | {vision_count} | {pct(vision_count):.1f}% |
| Take Food | {take_food_count} | {pct(take_food_count):.1f}% |
| Take Stone | {take_stone_count} | {pct(take_stone_count):.1f}% |
| Set Stone | {set_stone_count} | {pct(set_stone_count):.1f}% |
| Broadcast Radio | {broadcast_count} | {pct(broadcast_count):.1f}% |
| Fork Allied Slots | {fork_count} | {pct(fork_count):.1f}% |
| Incantations Attempted | {incant_count} | {pct(incant_count):.1f}% |

**Incantations Succeeded**: {stats["incantations_succeeded"]} / {stats["incantations_attempted"]} (Success rate: {(stats["incantations_succeeded"] / stats["incantations_attempted"] * 100) if stats["incantations_attempted"] > 0 else 0:.1f}%)
**Starved Deaths**: {stats["deaths"]}
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
