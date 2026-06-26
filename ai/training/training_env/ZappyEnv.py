from typing import Union

import gymnasium as gym
import numpy as np
from gymnasium import spaces
from src.client.ai_client import ZappyAiClient
from src.client.lib_client import ZappyLibClient
from src.utils import Inventory

# Import and re-export the modularized components for backwards compatibility
from training.training_env.actions import ControllerAction, ZappyAction
from training.training_env.broadcast import BroadcastDict, BroadcastHandler
from training.training_env.env_modes import LibZappyEnv, NetworkZappyEnv


class ObservationZappyEnv:
    client: Union[ZappyAiClient, ZappyLibClient, None] = None

    def _get_real_observation(self):
        """
        Retrieves the player's inventory and vision from the server and maps them
        into the 657-dimensional numpy array used for neural network input.
        """
        assert self.client is not None, "Client is not connected"

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

        # Parse Inventory
        inv = self.client.inventory()
        if isinstance(inv, str) and inv.startswith("[") and inv.endswith("]"):
            items = inv.strip("[]").split(",")
            for item in items:
                parts = item.strip().split(" ")
                if len(parts) == 2:
                    name = parts[0]
                    quantity = int(parts[1])
                    if name in resources:
                        idx = resources.index(name) - 1
                        if idx >= 0:
                            obs[idx] = quantity
            obs[656] = obs[0]
        elif isinstance(inv, Inventory):
            obs[0] = inv.food
            obs[1] = inv.linemate
            obs[2] = getattr(inv, "deraumere", 0)
            obs[3] = getattr(inv, "sibur", 0)
            obs[4] = getattr(inv, "mendiane", 0)
            obs[5] = getattr(inv, "phiras", 0)
            obs[6] = getattr(inv, "thystame", 0)
            obs[656] = inv.food

        # Parse Vision (Look)
        vision_list = self.client.look()
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
        obs[655] = self.client.level

        return obs


class ZappyEnv(ObservationZappyEnv, gym.Env):
    """
    Custom Gymnasium environment for the Zappy AI.
    Can be used in Network mode (TCP) or Library mode (Direct FFI).
    """

    def __init__(
        self,
        use_lib=True,
        port=4242,
        ip="127.0.0.1",
        team_name="team1",
        total_teams=2,
        **kwargs,
    ):
        super(ZappyEnv, self).__init__()

        # Dynamic team generator: ["team1", "team2", "team3"...]
        generated_teams = [team_name]
        for i in range(2, total_teams + 1):
            generated_teams.append(f"team{i}")

        if use_lib:
            self.mode = LibZappyEnv(self, teams=generated_teams, **kwargs)
        else:
            self.mode = NetworkZappyEnv(self, port=port, ip=ip, team_name=team_name)

        self.client = None
        self.action_space = spaces.Discrete(24)
        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

        self.broadcast_handler = BroadcastHandler(
            team_name=team_name, secret_key="ZAPPY_SEC"
        )

    def reset(self, *, seed=None, options=None):
        obs, info = self.mode.reset(seed=seed, options=options)
        self.client = self.mode.client
        return obs, info

    def close(self):
        self.mode.close()

    def step(self, action):
        """
        Executes a single step mapping PPO integers (0-23) to dynamic Zappy commands.
        """
        reward = -0.01
        terminated = False
        truncated = False
        response = None
        zappy_action = None
        item_target = None

        if self.client is None:
            return np.zeros(657, dtype=np.int32), 0.0, True, False, {}

        ZAPPY_ITEMS = [
            "food",
            "linemate",
            "deraumere",
            "sibur",
            "mendiane",
            "phiras",
            "thystame",
        ]

        try:
            bot_action = ControllerAction(int(action))
        except ValueError:
            bot_action = None

        pending_messages = []
        if isinstance(self.client, ZappyLibClient):
            pending_messages = self.client.get_unread_messages()

        best_heuristic = {"score": 0, "task": "IGNORE", "dir": 0}

        for msg in pending_messages:
            direction = msg.get("dir", 0)
            text = msg.get("text", "")
            heuristic = self.broadcast_handler.calculate_heuristic(direction, text)
            if heuristic["score"] > best_heuristic["score"]:
                best_heuristic = heuristic
                best_heuristic["dir"] = direction

        match bot_action:
            case ControllerAction.FORWARD:
                response = self.client.forward()
                zappy_action = ZappyAction.FORWARD
            case ControllerAction.LEFT:
                response = self.client.left()
                zappy_action = ZappyAction.LEFT
            case ControllerAction.RIGHT:
                response = self.client.right()
                zappy_action = ZappyAction.RIGHT
            case ControllerAction.LOOK:
                response = self.client.look()
                zappy_action = ZappyAction.LOOK
            case ControllerAction.INVENTORY:
                response = self.client.inventory()
                zappy_action = ZappyAction.INVENTORY
            case ControllerAction.BROADCAST:
                inv = self.client.inventory()

                # Do we have the required stones to reach at least Level 3?
                has_stones = (
                    isinstance(inv, Inventory)
                    and inv.linemate >= 1
                    and inv.deraumere >= 1
                    and inv.sibur >= 1
                )

                if self.client.level >= 2 and has_stones:
                    msg_to_send = self.broadcast_handler.build_message(
                        BroadcastDict.INCANT
                    )
                elif isinstance(inv, Inventory) and inv.food < 5:
                    msg_to_send = self.broadcast_handler.build_message(
                        BroadcastDict.FIND, "food"
                    )
                else:
                    msg_to_send = self.broadcast_handler.build_message(
                        BroadcastDict.FIND, "stones"
                    )

                response = self.client.broadcast(msg_to_send)
                zappy_action = ZappyAction.BROADCAST

            case ControllerAction.CONNECT_NBR:
                response = self.client.connect_nbr()
                zappy_action = ZappyAction.CONNECT_NBR
            case ControllerAction.FORK:
                response = self.client.fork()
                zappy_action = ZappyAction.FORK
            case ControllerAction.EJECT:
                response = self.client.eject()
                zappy_action = ZappyAction.EJECT
            case _ if (
                bot_action is not None
                and ControllerAction.TAKE_FOOD
                <= bot_action
                <= ControllerAction.TAKE_THYSTAME
            ):
                item_target = ZAPPY_ITEMS[bot_action - ControllerAction.TAKE_FOOD]
                response = self.client.take(item_target)
                zappy_action = ZappyAction.TAKE
            case _ if (
                bot_action is not None
                and ControllerAction.SET_FOOD
                <= bot_action
                <= ControllerAction.SET_THYSTAME
            ):
                item_target = ZAPPY_ITEMS[bot_action - ControllerAction.SET_FOOD]
                response = self.client.set(item_target)
                zappy_action = ZappyAction.SET
            case ControllerAction.INCANTATION:
                response = self.client.incantation()
                zappy_action = ZappyAction.INCANTATION
            case _:
                reward = -0.5

        # 3. Reward processing
        if self.client.is_dead or response == "dead" or response is None:
            terminated = True
            reward = -100.0
        elif response == "ok":
            base_rewards = {
                ZappyAction.FORWARD: 0.1,  # BUFF: Increased reward for walking (Anti-Casino)
                ZappyAction.LEFT: 0.02,
                ZappyAction.RIGHT: 0.02,
                ZappyAction.LOOK: 0.1,  # BUFF: Increased reward for looking around
                ZappyAction.INVENTORY: 0.0,
                ZappyAction.BROADCAST: 0.0,
                ZappyAction.CONNECT_NBR: 0.0,
                ZappyAction.FORK: -2.0,
                ZappyAction.EJECT: 0.0,
                ZappyAction.SET: -0.5,
                ZappyAction.INCANTATION: 0.0,
            }
            if zappy_action is not None:
                reward += base_rewards.get(zappy_action, 0.0)

            if best_heuristic["score"] >= 50:
                target_dir = best_heuristic["dir"]
                task = best_heuristic["task"]
                ideal_actions = []
                if task == "MOVE_TO_DIR":
                    if target_dir in [1, 2, 8]:
                        ideal_actions = [ZappyAction.FORWARD]
                    elif target_dir in [3, 4, 5]:
                        ideal_actions = [ZappyAction.LEFT]
                    elif target_dir in [6, 7]:
                        ideal_actions = [ZappyAction.RIGHT]
                    elif target_dir == 0:
                        ideal_actions = [
                            ZappyAction.INCANTATION,
                            ZappyAction.TAKE,
                            ZappyAction.LOOK,
                        ]

                elif task == "FLEE_FROM_DIR":
                    if target_dir in [1, 2, 8]:
                        ideal_actions = [ZappyAction.LEFT, ZappyAction.RIGHT]
                    elif target_dir in [3, 4, 5]:
                        ideal_actions = [ZappyAction.RIGHT, ZappyAction.FORWARD]
                    elif target_dir in [6, 7]:
                        ideal_actions = [ZappyAction.LEFT, ZappyAction.FORWARD]
                    elif target_dir == 0:
                        ideal_actions = [ZappyAction.FORWARD]

                if zappy_action in ideal_actions:
                    reward += 3.0  # reward for listening to the radio
                else:
                    reward -= 0.5  # Penalty for ignoring teammates

            if zappy_action == ZappyAction.TAKE:
                inv = self.client.inventory()
                if item_target == "food":
                    if isinstance(inv, Inventory) and inv.food >= 15:
                        reward -= 0.5
                    else:
                        reward += 2.0
                else:
                    if isinstance(inv, Inventory) and isinstance(item_target, str):
                        stone_quantity = getattr(inv, item_target, 0)
                        if stone_quantity < 5:
                            reward += 4.0
                        else:
                            reward -= 0.5

        elif response == "ko":
            # ANTI-CASINO SYSTEM
            # Penalize spamming the 'TAKE' button on empty tiles strictly,
            # while keeping regular mistake penalties low
            if zappy_action == ZappyAction.TAKE:
                reward -= 0.5
            else:
                reward -= 0.1

        elif isinstance(response, str) and response.startswith("Current level:"):
            reward += 100.0

        if not terminated:
            try:
                observation = self._get_real_observation()
            except Exception as e:
                print(f"[ENV] Error: {e}")
                observation = np.zeros(657, dtype=np.int32)
                terminated = True
                reward = -100.0
        else:
            observation = np.zeros(657, dtype=np.int32)

        info = {}
        return observation, reward, terminated, truncated, info

    @property
    def player_level(self) -> int:
        return self.client.level if self.client is not None else 1
