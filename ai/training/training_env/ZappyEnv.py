from typing import Union
import logging
import gymnasium as gym
import numpy as np
from gymnasium import spaces
from src.client.lib_client import ZappyLibClient
from src.utils import Inventory, ELEVATION_TABLE

# Import and re-export the modularized components for backwards compatibility
from training.training_env.actions import ControllerAction, ZappyAction
from training.training_env.broadcast import BroadcastDict, BroadcastHandler
from training.training_env.env_modes import LibZappyEnv

# Suppress verbose decision-making logs from teammate bots in subprocesses
logging.getLogger("zappy_ai").setLevel(logging.CRITICAL)


class ObservationZappyEnv:
    client: Union[ZappyLibClient, None] = None

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
        team_name="team1",
        total_teams=2,
        **kwargs,
    ):
        super(ZappyEnv, self).__init__()

        # Dynamic team generator: ["team1", "team2", "team3"...]
        generated_teams = [team_name]
        for i in range(2, total_teams + 1):
            generated_teams.append(f"team{i}")

        self.mode = LibZappyEnv(self, teams=generated_teams, **kwargs)

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
        self.turns_elapsed = 0
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
                ZappyAction.CONNECT_NBR: 0.0,
                ZappyAction.EJECT: 0.0,
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
                        reward += (
                            0.0  # No reward for excess food hoarding (focus on stones)
                        )
                    else:
                        reward += 2.0  # Large positive reward for survival
                else:
                    if isinstance(inv, Inventory) and isinstance(item_target, str):
                        req = ELEVATION_TABLE.get(self.client.level)
                        needed = getattr(req, item_target, 0) if req is not None else 0
                        current_quantity = getattr(inv, item_target, 0)
                        if current_quantity <= needed:
                            reward += 4.0
                        else:
                            reward -= 0.2

            elif zappy_action == ZappyAction.SET:
                if item_target == "food":
                    reward -= 2.0
                elif isinstance(item_target, str):
                    inv = self.client.inventory()
                    if isinstance(inv, Inventory):
                        req = ELEVATION_TABLE.get(self.client.level)
                        needed = getattr(req, item_target, 0) if req is not None else 0
                        current_quantity = getattr(inv, item_target, 0)
                        if current_quantity < needed:
                            reward -= 4.0
                        else:
                            reward += 0.2

            elif zappy_action == ZappyAction.FORK:
                slots = self.client.connect_nbr()
                req = ELEVATION_TABLE.get(self.client.level)
                if (
                    req is not None
                    and req.players > 1
                    and isinstance(slots, int)
                    and slots == 0
                ):
                    reward += 2.0
                else:
                    reward -= 2.0

            elif zappy_action == ZappyAction.BROADCAST:
                inv = self.client.inventory()
                req = ELEVATION_TABLE.get(self.client.level)
                if req is not None and isinstance(inv, Inventory):
                    has_stones = (
                        inv.linemate >= req.linemate
                        and inv.deraumere >= req.deraumere
                        and inv.sibur >= req.sibur
                        and inv.mendiane >= req.mendiane
                        and inv.phiras >= req.phiras
                        and inv.thystame >= req.thystame
                    )
                    if has_stones and req.players > 1:
                        reward += 2.0
                    elif inv.food < 5:
                        reward += 0.5
                    else:
                        reward -= 0.1
                else:
                    reward -= 0.1

        elif response == "ko":
            # ANTI-CASINO SYSTEM
            # Penalize spamming the 'TAKE' button on empty tiles strictly,
            # while keeping regular mistake penalties low
            if zappy_action == ZappyAction.TAKE:
                reward -= 0.5
            elif zappy_action == ZappyAction.INCANTATION:
                reward -= 10.0
            else:
                reward -= 0.1

        elif isinstance(response, str) and response.startswith("Current level:"):
            reward += 100.0 * self.client.level

        self.turns_elapsed += 1

        if not terminated:
            # Run background teammate bots once every 5 turns to allow training multi-agent coordination
            # without bloating the server ticking rate and starving the players instantly.
            if (
                hasattr(self.mode, "teammate_clients")
                and self.mode.teammate_clients
                and (self.turns_elapsed % 5 == 0)
            ):
                from src.strategy.decision_making import take_decision
                import logging

                logging.getLogger("zappy_ai").setLevel(logging.CRITICAL)

                for teammate in self.mode.teammate_clients:
                    if not teammate.is_dead:
                        try:
                            # Periodically send a COME beacon if teammate is Level 2
                            # to teach the agent to follow the radio signal
                            if teammate.level >= 2 and (self.turns_elapsed % 10 == 0):
                                teammate.broadcast("COME")
                            take_decision(teammate)
                        except Exception:
                            teammate.is_dead = True

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
