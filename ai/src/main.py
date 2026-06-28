#!/usr/bin/env python3

import sys
import os
import argparse
import numpy as np

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
)

from src.client import ZappyAiClient
from src.strategy import run_client, run_manual, run_llm

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


def run_hybrid_ai(client, team_name, model_path):
    """heuristic helps ai decisions."""
    from src.strategy.state_machine import get_hybrid_action
    from training.training_env.ZappyEnv import parse_look

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

        try:
            look_resp = client.look()
            vision_list = (
                parse_look(look_resp)
                if isinstance(look_resp, str) and look_resp.startswith("[")
                else look_resp
            )
        except Exception:
            vision_list = []

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

        # Check if flee condition is active
        if best_h["score"] == 50 and best_h["task"] == "FLEE_FROM_DIR":
            action = int(ControllerAction.RIGHT)
        else:
            best_dir = best_h["dir"] if best_h["score"] >= 70 else 0
            best_text = best_h["text"] if best_h["score"] >= 70 else ""

            action, _ = get_hybrid_action(
                client=client,
                parsed_inv=inv,
                vision_list=vision_list,
                current_level=current_level,
                best_dir=best_dir,
                best_text=best_text,
                model=model,
                obs=obs,
            )

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
        "-llm", "--llm", action="store_true", help="starts the game in LLM mode"
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
            if args.llm:
                run_llm(client)
            elif args.manual:
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
