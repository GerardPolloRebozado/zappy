import ctypes
import os
from pathlib import Path
from src.utils import parse_look, parse_broadcast, Inventory


class ZappyLib:
    def __init__(self, lib_path=None):
        if lib_path is None:
            lib_path = self._resolve_lib_path()

        self.lib = ctypes.CDLL(lib_path)

        # Define function signatures
        self.lib.zappy_init.argtypes = [
            ctypes.c_uint32,
            ctypes.c_uint32,
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_size_t,
            ctypes.c_uint32,
        ]
        self.lib.zappy_init.restype = ctypes.c_void_p

        self.lib.zappy_free.argtypes = [ctypes.c_void_p]

        self.lib.zappy_tick.argtypes = [ctypes.c_void_p, ctypes.c_uint64]

        self.lib.zappy_add_player.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self.lib.zappy_add_player.restype = ctypes.c_uint32

        self.lib.zappy_send_command.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint32,
            ctypes.c_char_p,
        ]

        self.lib.zappy_get_response.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
        self.lib.zappy_get_response.restype = ctypes.c_char_p

        self.lib.zappy_free_string.argtypes = [ctypes.c_char_p]

    def _resolve_lib_path(self):
        base_dir = Path(__file__).resolve().parents[3]

        # Look for the library in common locations
        suffix = ".dylib" if os.uname().sysname == "Darwin" else ".so"
        possible_locations = [
            base_dir / "server" / "target" / "debug" / f"libzappy_engine{suffix}",
            base_dir / "server" / "target" / "release" / f"libzappy_engine{suffix}",
            base_dir / f"libzappy_engine{suffix}",
        ]

        for path in possible_locations:
            if path.is_file():
                return str(path)

        raise FileNotFoundError(
            f"zappy_engine library not found ({suffix}). Please compile the server with 'cargo build'."
        )


class ZappyLibClient:
    def __init__(self, lib, server_ptr, player_id, freq):
        self.lib = lib
        self.server_ptr = server_ptr
        self.player_id = player_id
        self.freq = freq
        self.is_dead = False
        self.level = 1
        self.messages = []

    def _tick(self, ms):
        self.lib.zappy_tick(self.server_ptr, ms)

    def wait_for_response(self):
        # In headless mode, we might need to tick the engine to advance tasks
        # Each tick we check for a response.
        # Max timeout to prevent infinite loops if something goes wrong
        max_ticks = 100000
        ticks = 0
        while ticks < max_ticks:
            resp_ptr = self.lib.zappy_get_response(self.server_ptr, self.player_id)
            if resp_ptr:
                resp = ctypes.string_at(resp_ptr).decode("utf-8")
                self.lib.zappy_free_string(resp_ptr)

                if resp == "dead":
                    self.is_dead = True
                    return "dead"
                if resp.startswith("message"):
                    parsed = parse_broadcast(resp)
                    if parsed:
                        self.messages.append(parsed)
                    continue
                if resp.startswith("Current level:"):
                    try:
                        self.level = int(resp.split(":")[1].strip())
                    except (ValueError, IndexError):
                        pass
                    return resp
                return resp

            # Tick by 1ms (very granular for training, maybe more?)
            self._tick(1)
            ticks += 1

        return None

    def forward(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Forward\n")
        return self.wait_for_response()

    def right(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Right\n")
        return self.wait_for_response()

    def left(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Left\n")
        return self.wait_for_response()

    def look(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Look\n")
        resp = self.wait_for_response()
        return parse_look(resp) if resp and resp.startswith("[") else resp

    def inventory(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Inventory\n")
        resp = self.wait_for_response()
        return Inventory.from_string(resp) if resp and resp.startswith("[") else resp

    def broadcast(self, text):
        cmd = f"Broadcast {text}\n".encode("utf-8")
        self.lib.zappy_send_command(self.server_ptr, self.player_id, cmd)
        return self.wait_for_response()

    def connect_nbr(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Connect_nbr\n")
        resp = self.wait_for_response()
        try:
            return int(resp) if resp else None
        except ValueError:
            return resp

    def fork(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Fork\n")
        return self.wait_for_response()

    def eject(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Eject\n")
        return self.wait_for_response()

    def take(self, obj):
        cmd = f"Take {obj}\n".encode("utf-8")
        self.lib.zappy_send_command(self.server_ptr, self.player_id, cmd)
        return self.wait_for_response()

    def set(self, obj):
        cmd = f"Set {obj}\n".encode("utf-8")
        self.lib.zappy_send_command(self.server_ptr, self.player_id, cmd)
        return self.wait_for_response()

    def incantation(self):
        self.lib.zappy_send_command(self.server_ptr, self.player_id, b"Incantation\n")
        first_resp = self.wait_for_response()
        if first_resp == "Elevation underway":
            return self.wait_for_response()
        return first_resp

    def close(self):
        # Handled by ZappyLibManager
        pass
