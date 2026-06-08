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

        self.observation_space = spaces.Box(
            low=0, high=10000, shape=(657,), dtype=np.int32
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.server_manager.start()

        self.client = ZappyAiClient(self.port, self.team_name, self.ip)
        self.client.connect()
        observation = self._get_real_observation()
        info = {}
        return observation, info

    def step(self, action):
        reward = 0.0
        terminated = False
        truncated = False
        response = None

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
            )  # TODO: he AI could choose whatever it want to drop
        elif action == 18:
            response = self.client.incantation()
        else:
            reward = -0.5

        if self.client.is_dead or response == "dead":
            terminated = True
            reward = -100.0
        elif response == "ok":
            reward = 1.0
        elif response == "ko":
            reward = -0.1

        if not terminated:
            observation = self._get_real_observation()
        else:
            observation = np.zeros(657, dtype=np.int32)

        info = {}
        return observation, reward, terminated, truncated, info

    def _get_real_observation(self):
        obs = np.zeros(657, dtype=np.int32)

        inv = self.client.inventory()
        res = inv.split(" ")
        i = 0
        for item in res:
            obs[i] = int(item)
            i += 1
        if inv and type(inv) is not str:
            pass

        vision_list = self.client.look()
        if vision_list and type(vision_list) is list:
            pass

        obs[655] = self.client.level
        obs[656] = inv.food

        return obs

    def close(self):
        if self.client:
            self.client.close()
        self.server_manager.stop()
