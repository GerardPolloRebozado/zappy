from enum import Enum
import gymnasium as gym
from gymnasium import spaces
import numpy as np
from training.training_env.server_manager import ServerManager
from src.client.ai_client import ZappyAiClient

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


class ZappyEnv(gym.Env):
    """
    Custom Gymnasium environment for the Zappy AI.
    Handles the translation between Stable Baselines 3 actions and the Zappy server protocol.
    """

    def __init__(self, port=4242, ip="127.0.0.1", team_name="TeamAI"):
        """
        Initializes the Zappy Environment.
        """
        super(ZappyEnv, self).__init__()
        self.ip = ip
        self.port = port
        self.team_name = team_name
        self.server_manager = ServerManager(port=port)
        self.client = None

        self.action_space = spaces.Discrete(24)

        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

    def reset(self, seed=None, options=None):
        """
        Resets the environment for a new episode.
        Restarts the Rust server, connects the AI client, and fetches the initial observation.
        """
        super().reset(seed=seed)

        if self.client:
            self.client.close()

        self.server_manager.start()

        self.port = self.server_manager.port

        self.client = ZappyAiClient(self.port, self.team_name, self.ip)
        self.client.connect()
        observation = self._get_real_observation()
        info = {}
        return observation, info

    def step(self, action):
        """
        Executes a single step mapping PPO integers (0-23) to dynamic Zappy commands.
        """
        action = int(action)
        reward = -0.01
        terminated = False
        truncated = False
        response = None

        zappy_action = None
        item_target = None

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
            match action:
                case 0:
                    response = self.client.forward()
                    zappy_action = ZappyAction.FORWARD
                case 1:
                    response = self.client.left()
                    zappy_action = ZappyAction.LEFT
                case 2:
                    response = self.client.right()
                    zappy_action = ZappyAction.RIGHT
                case 3:
                    response = self.client.look()
                    zappy_action = ZappyAction.LOOK
                case 4:
                    response = self.client.inventory()
                    zappy_action = ZappyAction.INVENTORY
                case 5:
                    response = self.client.broadcast("Hola")
                    zappy_action = ZappyAction.BROADCAST
                case 6:
                    response = self.client.connect_nbr()
                    zappy_action = ZappyAction.CONNECT_NBR
                case 7:
                    response = self.client.fork()
                    zappy_action = ZappyAction.FORK
                case 8:
                    response = self.client.eject()
                    zappy_action = ZappyAction.EJECT

                case _ if 9 <= action <= 15:
                    item_target = ZAPPY_ITEMS[action - 9]
                    response = self.client.take(item_target)
                    zappy_action = ZappyAction.TAKE

                case _ if 16 <= action <= 22:
                    item_target = ZAPPY_ITEMS[action - 16]
                    response = self.client.set(item_target)
                    zappy_action = ZappyAction.SET

                case 23:
                    response = self.client.incantation()
                    zappy_action = ZappyAction.INCANTATION

                case _:
                    reward = -0.5

        except BrokenPipeError:
            print("[ENV] BrokenPipe: Dead Player")
            response = "dead"
            self.client.is_dead = True
        except Exception as e:
            print(f"[ENV] Network: {e}")
            response = "dead"
            self.client.is_dead = True

        if self.client.is_dead or response == "dead" or response is None:
            terminated = True
            reward = -100.0

        elif response == "ok":
            base_rewards = {
                ZappyAction.FORWARD: 0.0,
                ZappyAction.LEFT: 0.0,
                ZappyAction.RIGHT: 0.0,
                ZappyAction.LOOK: 0.0,
                ZappyAction.INVENTORY: 0.0,
                ZappyAction.BROADCAST: 0.0,
                ZappyAction.CONNECT_NBR: 0.0,
                ZappyAction.FORK: 50.0,
                ZappyAction.EJECT: 10.0,
                ZappyAction.SET: 5.0,
                ZappyAction.INCANTATION: 0.0,
            }
            reward += base_rewards.get(zappy_action, 0.0)

            if zappy_action == ZappyAction.TAKE:
                inv = self.client.inventory()

                if item_target == "food":
                    if hasattr(inv, "food") and inv.food >= 15:
                        reward += 0.0
                    else:
                        reward += 4.0
                else:
                    if getattr(inv, item_target, 0) >= 1:
                        reward += 10.0
                    else:
                        reward += 2.0

        elif response == "ko":
            reward = -1.0

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

    def _get_real_observation(self):
        """
        Retrieves the player's inventory and vision from the server and maps them
        into the 657-dimensional numpy array used for neural network input.
        """
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
        elif hasattr(inv, "food"):
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

    def close(self):
        """
        Closes the environment, disconnects the socket, and stops the server process.
        """
        if self.client:
            self.client.close()
        self.server_manager.stop()
