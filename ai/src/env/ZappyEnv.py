import gymnasium as gym
from gymnasium import spaces
import numpy as np
from src.env.server_manager import ServerManager
from src.client.ai_client import ZappyAiClient


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

        self.action_mapping = {
            1: "Forward",
            3: "Left",
            7: "Right",
            10: "Look",
            11: "Inventory",
            12: "Broadcast",
            13: "Connect_nbr",
            14: "Fork",
            15: "Eject",
            16: "Take",
            17: "Set",
            18: "Incantation",
        }

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
        Executes a single step in the environment.
        Translates the discrete action into a server command, evaluates the response,
        and calculates the appropriate reward. Catches BrokenPipeErrors if the server closes.

        :param action: An integer representing the chosen action.
        :return: A tuple (observation, reward, terminated, truncated, info).
        """
        reward = 0.0
        terminated = False
        truncated = False
        response = None

        try:
            if action == 1:
                response = self.client.forward()
            elif action == 3:
                response = self.client.left()
            elif action == 7:
                response = self.client.right()
            elif action == 10:
                response = self.client.look()
            elif action == 11:
                response = self.client.inventory()
            elif action == 12:
                response = self.client.broadcast("Hola")
            elif action == 13:
                response = self.client.connect_nbr()
            elif action == 14:
                response = self.client.fork()
            elif action == 15:
                response = self.client.eject()
            elif action == 16:
                response = self.client.take(
                    "food"
                )  # TODO: the AI could choose whatever it want
            elif action == 17:
                response = self.client.set(
                    "food"
                )  # TODO: the AI could choose whatever it want to drop
            elif action == 18:
                # response = self.client.incantation()
                response = "ko"
                reward = -1.0
            else:
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
