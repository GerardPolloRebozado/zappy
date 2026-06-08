import gymnasium as gym
from gymnasium import spaces
import numpy as np
from server_manager import ServerManager
from src.client.ai_client import ZappyAiClient


class ZappyEnv(gym.Env):
    def __init__(self, port=4242, ip="127.0.0.1", team_name="TeamAI"):
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

        # shape = 81 vision tiles * 8 posibles recursos + 7 inventario + nivel + vida
        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.server_manager.start()
        self.client = ZappyAiClient(self.port, self.team_name, self.ip)
        self.client.connect()
        self.client.receive_line()
        self.client.receive_line()
        self.client.receive_line()
        observation = self._get_real_observation()
        info = {}

        return observation, info

    def step(self, action):
        command = self.action_mapping.get(action, None)

        reward = 0.0
        terminated = False
        truncated = False

        if command is not None:
            self.client.send(f"{command}\n")

            response = self.client.receive().strip()

            if response == "dead":
                terminated = True
                reward = -100.0
            elif response == "ok":
                reward = 1.0
            elif response == "ko":
                reward = -0.1
        else:
            reward = -0.5

        if not terminated:
            observation = self._get_real_observation()
        else:
            observation = np.zeros(657, dtype=np.int32)

        info = {}
        return observation, reward, terminated, truncated, info

    def _get_real_observation(self):
        obs = np.zeros(657, dtype=np.int32)
        self.client.send("Inventory\n")
        self.client.receive_line()
        # TODO: string invetory to maths

        self.client.send("Look\n")
        self.client.receive()

        # TODO: vision logic

        return obs

    def close(self):
        if self.client:
            pass
        self.server_manager.stop()
