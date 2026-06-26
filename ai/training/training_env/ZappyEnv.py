import ctypes
from enum import Enum, IntEnum
from typing import Union

import gymnasium as gym
import numpy as np
from gymnasium import spaces
from src.client.ai_client import ZappyAiClient
from src.client.lib_client import ZappyLib, ZappyLibClient
from src.utils import Inventory

from training.training_env.server_manager import ServerManager

"""
---------------------------------------
| rollout/                |           |
|    ep_len_mean          | 2.67e+04  |
|    ep_rew_mean          | 1.31e+06  |
| time/                   |           |
|    fps                  | 453       |
|    iterations           | 489       |
|    time_elapsed         | 2209      |
|    total_timesteps      | 1001472   |
| train/                  |           |
|    approx_kl            | 0.0       |
|    clip_fraction        | 0         |
|    clip_range           | 0.2       |
|    entropy_loss         | -0.000338 |
|    explained_variance   | 0         |
|    learning_rate        | 0.0003    |
|    loss                 | 5.07e+04  |
|    n_updates            | 4880      |
|    policy_gradient_loss | 1.44e-05  |
|    value_loss           | 1.04e+05  |
---------------------------------------
"""


class ControllerAction(IntEnum):
    FORWARD = 0
    LEFT = 1
    RIGHT = 2
    LOOK = 3
    INVENTORY = 4
    BROADCAST = 5
    CONNECT_NBR = 6
    FORK = 7
    EJECT = 8
    TAKE_FOOD = 9
    TAKE_LINEMATE = 10
    TAKE_DERAUMERE = 11
    TAKE_SIBUR = 12
    TAKE_MENDIANE = 13
    TAKE_PHIRAS = 14
    TAKE_THYSTAME = 15
    SET_FOOD = 16
    SET_LINEMATE = 17
    SET_DERAUMERE = 18
    SET_SIBUR = 19
    SET_MENDIANE = 20
    SET_PHIRAS = 21
    SET_THYSTAME = 22
    INCANTATION = 23


class BroadcastDict(Enum):
    COME = "COME"
    FIND = "FIND"
    DONT_COME = "DCOME"
    INCANT = "INCANT"


class BroadcastHandler:
    def __init__(self, team_name, secret_key):
        self.team_name = team_name
        self.secret_key = secret_key

    def build_message(self, intent: BroadcastDict, params: str = "") -> str:
        """Build a safe message using the team's secret key."""
        return f"{self.team_name}|{self.secret_key}|{intent.value}|{params}"

    def parse_message(self, raw_message: str):
        """Decypher the message to ensure it belongs to our team."""
        parts = raw_message.split("|")
        if (
            len(parts) >= 3
            and parts[0] == self.team_name
            and parts[1] == self.secret_key
        ):
            return parts[2], parts[3] if len(parts) > 3 else ""
        return None, None

    def calculate_heuristic(self, direction: int, raw_message: str) -> dict:
        """Heuristic calculations to determine the importance of the broadcast."""
        action, params = self.parse_message(raw_message)

        if not action:
            return {"score": 0, "task": "IGNORE"}

        match action:
            case BroadcastDict.INCANT.value:
                return {"score": 100, "task": "MOVE_TO_DIR", "dir": direction}
            case BroadcastDict.COME.value:
                return {"score": 70, "task": "MOVE_TO_DIR", "dir": direction}
            case BroadcastDict.DONT_COME.value:
                return {"score": 50, "task": "FLEE_FROM_DIR", "dir": direction}
            case BroadcastDict.FIND.value:
                return {"score": 30, "task": "SEARCH_RESOURCE", "target": params}

        return {"score": 0, "task": "IGNORE"}


class ZappyAction(Enum):
    """
    Enum mapping actions to their corresponding Zappy commands logic.
    """

    FORWARD = "Forward"
    LEFT = "Left"
    RIGHT = "Right"
    LOOK = "Look"
    INVENTORY = "Inventory"
    BROADCAST = "Broadcast"
    CONNECT_NBR = "Connect_nbr"
    FORK = "Fork"
    EJECT = "Eject"
    TAKE = "Take"
    SET = "Set"
    INCANTATION = "Incantation"


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


class NetworkZappyEnv:
    def __init__(self, env, port=4242, ip="127.0.0.1", team_name="team1"):
        self.env = env
        self.ip = ip
        self.port = port
        self.team_name = team_name
        self.server_manager = ServerManager(port=port)
        self.client = None

    def reset(self, *, seed=None, options=None):
        if self.client:
            self.client.close()

        self.server_manager.start()
        self.port = self.server_manager.port

        self.client = ZappyAiClient(self.port, self.team_name, self.ip)
        self.client.connect()
        self.env.client = self.client

        observation = self.env._get_real_observation()
        info = {}
        return observation, info

    def close(self):
        if self.client:
            self.client.close()
        self.server_manager.stop()


class LibZappyEnv:
    def __init__(
        self,
        env,
        freq=100,
        teams=["team1", "team2"],
        clients_nb=10,
        **kwargs,
    ):
        self.env = env
        self.width = int(np.random.randint(10, 50))
        self.height = int(np.random.randint(10, 50))
        self.freq = freq
        self.teams = teams
        self.clients_nb = clients_nb
        self.zappy_lib = ZappyLib()
        self.server_ptr = None
        self.client = None

    def reset(self, *, seed=None, options=None):
        if self.server_ptr:
            self.zappy_lib.lib.zappy_free(self.server_ptr)

        num_teams = int(np.random.randint(1, 6))
        teams = [self.teams[0]] + [f"team{i}" for i in range(2, num_teams + 1)]

        self.width = int(np.random.randint(10, 25))
        self.height = int(np.random.randint(10, 25))
        area = self.width * self.height
        players_per_team = max(1, area // (50 * num_teams))
        clients_nb = players_per_team

        TeamArray = ctypes.c_char_p * num_teams
        team_ptrs = TeamArray(*[t.encode("utf-8") for t in teams])

        self.server_ptr = self.zappy_lib.lib.zappy_init(
            self.width, self.height, self.freq, team_ptrs, num_teams, clients_nb
        )

        # -1 player for allies that 1 of them is the ai
        for _ in range(players_per_team - 1):
            self.zappy_lib.lib.zappy_add_player(
                self.server_ptr, teams[0].encode("utf-8")
            )

        # Enemies
        for i in range(1, num_teams):
            for _ in range(players_per_team):
                self.zappy_lib.lib.zappy_add_player(
                    self.server_ptr, teams[i].encode("utf-8")
                )

        player_id = self.zappy_lib.lib.zappy_add_player(
            self.server_ptr, teams[0].encode("utf-8")
        )

        self.client = ZappyLibClient(
            self.zappy_lib.lib, self.server_ptr, player_id, self.freq
        )
        self.env.client = self.client

        # Consume initial auth responses ("ok", slots, map size)
        for _ in range(2):
            self.client.wait_for_response()

        observation = self.env._get_real_observation()
        info = {}
        return observation, info

    def close(self):
        if self.server_ptr:
            self.zappy_lib.lib.zappy_free(self.server_ptr)
            self.server_ptr = None


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
