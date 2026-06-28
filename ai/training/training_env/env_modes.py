import ctypes
import numpy as np
from src.client.lib_client import ZappyLib, ZappyLibClient


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

        self.width = int(np.random.randint(10, 31))
        self.height = int(np.random.randint(10, 31))
        players_per_team = int(np.random.randint(1, 4))
        clients_nb = players_per_team

        TeamArray = ctypes.c_char_p * num_teams
        team_ptrs = TeamArray(*[t.encode("utf-8") for t in teams])

        self.server_ptr = self.zappy_lib.lib.zappy_init(
            self.width, self.height, self.freq, team_ptrs, num_teams, clients_nb
        )

        # -1 player for allies that 1 of them is the ai
        self.teammate_clients = []
        for _ in range(players_per_team - 1):
            p_id = self.zappy_lib.lib.zappy_add_player(
                self.server_ptr, teams[0].encode("utf-8")
            )
            teammate = ZappyLibClient(
                self.zappy_lib.lib, self.server_ptr, p_id, self.freq
            )
            teammate.name = f"TeammateBot_{p_id}"
            self.teammate_clients.append(teammate)

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
        self.client.name = teams[0]
        self.env.client = self.client

        # Consume initial auth responses ("ok", slots, map size) for teammates
        for teammate in self.teammate_clients:
            for _ in range(2):
                teammate.wait_for_response()

        for _ in range(2):
            self.client.wait_for_response()

        observation = self.env._get_real_observation()
        info = {}
        return observation, info

    def close(self):
        if self.server_ptr:
            self.zappy_lib.lib.zappy_free(self.server_ptr)
            self.server_ptr = None
