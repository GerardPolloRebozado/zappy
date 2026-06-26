import ctypes
import numpy as np
from src.client.ai_client import ZappyAiClient
from src.client.lib_client import ZappyLib, ZappyLibClient
from training.training_env.server_manager import ServerManager


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
