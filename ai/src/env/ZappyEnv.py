from enum import IntEnum
import gymnasium as gym
from gymnasium import spaces
import numpy as np
from src.env.server_manager import ServerManager
from src.client.ai_client import ZappyAiClient


class ZappyAction(IntEnum):
    """
    Enum mapping actions to their corresponding Zappy commands.
    """

    FORWARD = 1
    LEFT = 3
    RIGHT = 7
    LOOK = 10
    INVENTORY = 11
    BROADCAST = 12
    CONNECT_NBR = 13
    FORK = 14
    EJECT = 15
    TAKE = 16
    SET = 17
    INCANTATION = 18


class ZappyEnv(gym.Env):
    """
    Custom Gymnasium environment for the Zappy AI.
    Handles the translation between Stable Baselines 3 actions and the Zappy server protocol.
    """

    def __init__(self, port=4242, ip="127.0.0.1", team_name="TeamAI"):
        """
        Initializes the Zappy Environment.

        :param port: The port number for the server.
        :param ip: The IP address of the server.
        :param team_name: The name of the AI team.
        """
        super(ZappyEnv, self).__init__()
        self.ip = ip
        self.port = port
        self.team_name = team_name
        self.server_manager = ServerManager(port=port)
        self.client = None

        self.action_space = spaces.Discrete(19)

        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

    def reset(self, seed=None, options=None):
        """
        Resets the environment for a new episode.
        Restarts the Rust server, connects the AI client, and fetches the initial observation.

        :param seed: Optional random seed.
        :param options: Optional configuration dictionary.
        :return: A tuple containing the initial observation and an info dictionary.
        """
        super().reset(seed=seed)
        self.server_manager.start()

        self.client = ZappyAiClient(self.port, self.team_name, self.ip)
        self.client.connect()
        observation = self._get_real_observation()
        info = {}
        return observation, info

    def step(self, action):
        """
        Executes a single step in the environment by converting the incoming

        integer action into a ZappyAction Enum.
        """
        reward = 0.0
        terminated = False
        truncated = False
        response = None

        try:
            zappy_action = ZappyAction(action)
        except ValueError:
            zappy_action = None

        try:
            match zappy_action:
                case ZappyAction.FORWARD:
                    response = self.client.forward()
                case ZappyAction.LEFT:
                    response = self.client.left()
                case ZappyAction.RIGHT:
                    response = self.client.right()
                case ZappyAction.LOOK:
                    response = self.client.look()
                case ZappyAction.INVENTORY:
                    response = self.client.inventory()
                case ZappyAction.BROADCAST:
                    response = self.client.broadcast("Hola")
                case ZappyAction.CONNECT_NBR:
                    response = self.client.connect_nbr()
                case ZappyAction.FORK:
                    response = self.client.fork()
                case ZappyAction.EJECT:
                    response = self.client.eject()
                case ZappyAction.TAKE:
                    response = self.client.take("food")  # TODO: dynamic item selection
                case ZappyAction.SET:
                    response = self.client.set("food")  # TODO: dynamic item selection
                case ZappyAction.INCANTATION:
                    response = "ko"
                    reward = -1.0
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

        if self.client.is_dead or response == "dead":
            terminated = True
            reward = -100.0
        elif response == "ok":
            reward = 1.0
        elif response == "ko":
            reward = -0.1

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

        :return: A 1D numpy array containing the parsed observation.
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
