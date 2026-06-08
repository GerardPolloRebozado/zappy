import gymnasium as gym
from gymnasium import spaces
import numpy as np
from server_manager import ServerManager


class ZappyEnv(gym.Env):
    def __init__(self, port=4242, ip="127.0.0.1"):
        super(ZappyEnv, self).__init__()
        self.ip = ip
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
        # shape = 81 vision tiles * 8 posible things in a tile + 7 inventory slots + lvl + food left
        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self.server_manager.start()

        # TODO: make handshake, fill observation values

        observation = np.zeros(657, dtype=np.int32)
        info = {}
        return observation, info

    def step(self, action):
        command = self.action_mapping.get(action, None)

        if command is not None:
            # TODO: send command and wait answer
            pass

        observation = np.zeros(657, dtype=np.int32)
        reward = 0.0
        terminated = False  # TODO: kill bot if its dead
        truncated = False
        info = {}

        return observation, reward, terminated, truncated, info

    def close(self):
        self.server_manager.stop()
