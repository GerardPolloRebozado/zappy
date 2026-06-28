#!/usr/bin/env python3

import sys
import os
import argparse
import numpy as np

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
)

from src.client import ZappyAiClient
from src.strategy import run_client, run_manual

try:
    from stable_baselines3 import PPO
    from training.training_env.ZappyEnv import (
        BroadcastHandler,
        ObservationZappyEnv,
        ControllerAction,
        BroadcastDict,
    )

    class EnvObserver(ObservationZappyEnv):
        def __init__(self, client):
            self.client = client

        def _get_real_observation(self):
            obs = super()._get_real_observation()
            obs[79] = 0
            return obs

    CAN_RUN_AI = True
except ImportError:
    CAN_RUN_AI = False


def execute_action(client, action_id, handler):
    """transforms the numbers of the AI into the commands of the server"""
    try:
        bot_action = ControllerAction(int(action_id))
    except ValueError:
        bot_action = ControllerAction.FORWARD
    items = ["food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

    match bot_action:
        case ControllerAction.FORWARD:
            client.forward()
        case ControllerAction.LEFT:
            client.left()
        case ControllerAction.RIGHT:
            client.right()
        case ControllerAction.LOOK:
            client.look()
        case ControllerAction.INVENTORY:
            client.inventory()
        case ControllerAction.CONNECT_NBR:
            client.connect_nbr()
        case ControllerAction.FORK:
            client.fork()
        case ControllerAction.EJECT:
            client.eject()
        case ControllerAction.INCANTATION:
            client.incantation()
        case ControllerAction.BROADCAST:
            inv = client.inventory()
            if client.level >= 2 and hasattr(inv, "food") and inv.food > 15:
                msg = handler.build_message(BroadcastDict.INCANT)
            elif hasattr(inv, "food") and inv.food < 5:
                msg = handler.build_message(BroadcastDict.FIND, "food")
            else:
                msg = handler.build_message(BroadcastDict.COME)
            client.broadcast(msg)
        case _ if (
            ControllerAction.TAKE_FOOD <= bot_action <= ControllerAction.TAKE_THYSTAME
        ):
            client.take(items[bot_action - ControllerAction.TAKE_FOOD])
        case _ if (
            ControllerAction.SET_FOOD <= bot_action <= ControllerAction.SET_THYSTAME
        ):
            client.set(items[bot_action - ControllerAction.SET_FOOD])


def get_survival_action(client, parsed_inv, vision_list, item_name="food"):
    import math as pymath

    # If we have a cached path, continue executing it
    if hasattr(client, "path_steps") and client.path_steps:
        return client.path_steps.pop(0)

    # Otherwise, find the nearest tile containing the target item
    if not isinstance(vision_list, list):
        return None

    target_tile = -1
    for i, tile in enumerate(vision_list):
        if i >= 81:
            break
        if isinstance(tile, str):
            entities = tile.strip().split(" ")
        else:
            entities = tile
        if item_name in entities:
            target_tile = i
            break

    if target_tile == -1:
        return None

    if target_tile == 0:
        action_name = f"TAKE_{item_name.upper()}"
        return int(getattr(ControllerAction, action_name))

    # Calculate path steps
    d = int(pymath.sqrt(target_tile))
    x = target_tile - (d**2 + d)
    path = [int(ControllerAction.FORWARD)] * d
    if x < 0:
        path.append(int(ControllerAction.LEFT))
        path.extend([int(ControllerAction.FORWARD)] * abs(x))
    elif x > 0:
        path.append(int(ControllerAction.RIGHT))
        path.extend([int(ControllerAction.FORWARD)] * x)

    action_name = f"TAKE_{item_name.upper()}"
    path.append(int(getattr(ControllerAction, action_name)))

    client.path_steps = path
    return client.path_steps.pop(0)


def has_stones_for_level(parsed_inv, vision_list, level):
    tile_0_stones = {
        "linemate": 0,
        "deraumere": 0,
        "sibur": 0,
        "mendiane": 0,
        "phiras": 0,
        "thystame": 0,
    }
    if isinstance(vision_list, list) and len(vision_list) > 0:
        tile_0 = vision_list[0]
        entities = tile_0.strip().split(" ") if isinstance(tile_0, str) else tile_0
        for entity in entities:
            if entity in tile_0_stones:
                tile_0_stones[entity] += 1

    inv_linemate = parsed_inv.linemate if parsed_inv is not None else 0
    inv_deraumere = getattr(parsed_inv, "deraumere", 0) if parsed_inv is not None else 0
    inv_sibur = getattr(parsed_inv, "sibur", 0) if parsed_inv is not None else 0

    total_linemate = inv_linemate + tile_0_stones["linemate"]
    total_deraumere = inv_deraumere + tile_0_stones["deraumere"]
    total_sibur = inv_sibur + tile_0_stones["sibur"]

    if level == 1:
        return total_linemate >= 1
    elif level == 2:
        return total_linemate >= 1 and total_deraumere >= 1 and total_sibur >= 1
    return True


def run_hybrid_ai(client, team_name, model_path):
    """heuristic helps ai decisions."""
    full_path = model_path if model_path.endswith(".zip") else f"{model_path}.zip"
    if not os.path.exists(full_path):
        print(f"[Error]: None module found {full_path}")
        return

    model = PPO.load(full_path)
    handler = BroadcastHandler(team_name=team_name, secret_key="ZAPPY_SEC")
    obs_gen = EnvObserver(client)

    while not client.is_dead:
        try:
            obs = obs_gen._get_real_observation()
        except Exception:
            obs = np.zeros(657, dtype=np.int32)

        try:
            current_level = client.level
            inv = client.inventory()
        except Exception:
            current_level = 1
            inv = None

        food_count = getattr(inv, "food", 10) if inv is not None else 10
        try:
            look_resp = client.look()
        except Exception:
            look_resp = []

        has_stones_for_lvl3 = False
        if inv is not None and hasattr(inv, "linemate"):
            has_stones_for_lvl3 = (
                inv.linemate >= 1 and inv.deraumere >= 1 and inv.sibur >= 1
            )

        messages = (
            client.get_unread_messages()
            if hasattr(client, "get_unread_messages")
            else []
        )
        best_h = {"score": 0, "task": "IGNORE", "dir": 0, "text": ""}
        for msg in messages:
            h = handler.calculate_heuristic(msg["dir"], msg["text"])
            if h["score"] > best_h["score"]:
                best_h = {**h, "dir": msg["dir"], "text": msg["text"]}

        override = False
        action = None

        # 1. Critical Starvation (food < 4): Find food first!
        if food_count < 4:
            if hasattr(client, "path_steps") and client.path_steps:
                if client.path_steps[-1] != int(ControllerAction.TAKE_FOOD):
                    client.path_steps = []
            survival_action = get_survival_action(client, inv, look_resp, "food")
            if survival_action is not None:
                override = True
                action = survival_action
            else:
                # No food in sight: Force movement to explore!
                override = True
                action = int(ControllerAction.FORWARD)

        # 2. Teammate Calling (COME): Walk towards teammate (only if food >= 4)
        elif (
            current_level == 2
            and best_h["score"] >= 70
            and best_h["dir"] != 0
            and food_count >= 4
        ):
            override = True
            client.path_steps = []
            if best_h["dir"] in [1, 2, 8]:
                action = int(ControllerAction.FORWARD)
            elif best_h["dir"] in [3, 4, 5]:
                action = int(ControllerAction.LEFT)
            else:
                action = int(ControllerAction.RIGHT)

        # 3. Teammate Calling (INCANT) and together: Drop stones (only if food >= 4)
        elif (
            current_level == 2
            and best_h["score"] >= 70
            and best_h["dir"] == 0
            and "INCANT" in best_h["text"]
            and food_count >= 4
        ):
            override = True
            client.path_steps = []
            if has_stones_for_lvl3:
                if inv.linemate >= 1:
                    action = int(ControllerAction.SET_LINEMATE)
                elif inv.deraumere >= 1:
                    action = int(ControllerAction.SET_DERAUMERE)
                elif inv.sibur >= 1:
                    action = int(ControllerAction.SET_SIBUR)
            else:
                action = int(ControllerAction.LOOK)

        # 4. Have all stones, Level 2, no caller: Broadcast coordinates (only if food >= 8)
        elif (
            current_level == 2
            and has_stones_for_lvl3
            and best_h["score"] < 70
            and food_count >= 8
        ):
            override = True
            client.path_steps = []
            action = int(ControllerAction.BROADCAST)

        # 5. Normal Hunger (food < 8): Walk to food
        elif food_count < 8:
            if hasattr(client, "path_steps") and client.path_steps:
                if client.path_steps[-1] != int(ControllerAction.TAKE_FOOD):
                    client.path_steps = []
            survival_action = get_survival_action(client, inv, look_resp, "food")
            if survival_action is not None:
                override = True
                action = survival_action
            else:
                override = True
                action = int(ControllerAction.FORWARD)

        # 6. Opportunistic Eating (food < 15): Take food only if visible in Look cone!
        elif (
            food_count < 15
            and get_survival_action(client, inv, look_resp, "food") is not None
        ):
            override = True
            action = get_survival_action(client, inv, look_resp, "food")

        elif best_h["score"] == 50 and best_h["task"] == "FLEE_FROM_DIR":
            override = True
            action = int(ControllerAction.RIGHT)

        if override and action is not None:
            execute_action(client, action, handler)
        else:
            action = None
            if current_level == 2:
                missing_stones = []
                if inv is not None:
                    if inv.linemate < 1:
                        missing_stones.append("linemate")
                    if inv.deraumere < 1:
                        missing_stones.append("deraumere")
                    if inv.sibur < 1:
                        missing_stones.append("sibur")

                for stone in missing_stones:
                    stone_action = get_survival_action(client, inv, look_resp, stone)
                    if stone_action is not None:
                        action = stone_action
                        break

            if action is None:
                client.path_steps = []  # Clear path
                action, _ = model.predict(obs, deterministic=True)
                action = int(action)
                if action == int(ControllerAction.INCANTATION):
                    if not has_stones_for_level(inv, look_resp, current_level):
                        action = int(ControllerAction.FORWARD)
            execute_action(client, action, handler)


def main():
    """
    Main function. Handles parameter parsing and start the client.
    :return: 0 or 84 on error
    """
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-p", type=int, dest="port", help="port number")
    parser.add_argument("-n", type=str, dest="name", help="name of the team")
    parser.add_argument("-ip", type=str, dest="ip", help="ip address")
    parser.add_argument("-h", "--help", action="help", help="show help message")
    parser.add_argument(
        "-m", "--manual", action="store_true", help="starts the game in manual mode"
    )
    parser.add_argument(
        "--ai", action="store_true", help="starts the game using the trained AI model"
    )
    parser.add_argument(
        "--model",
        type=str,
        default="zappy_ai_model",
        help="name of the trained model to load",
    )

    try:
        args = parser.parse_args()
        if args.port is None or args.name is None or args.ip is None:
            print("USAGE: ./zappy_ai -p port -n name -ip ip address")
            return 84

        client = ZappyAiClient(args.port, args.name, args.ip)
        if client.connect() == 0:
            if args.manual:
                run_manual(client)
            elif args.ai:
                run_hybrid_ai(client, args.name, args.model)
            else:
                run_client(client)
            return 0
        return 84
    except SystemExit as e:
        return 0 if e.code == 0 else 84
    except Exception:
        return 84


if __name__ == "__main__":
    sys.exit(main())
