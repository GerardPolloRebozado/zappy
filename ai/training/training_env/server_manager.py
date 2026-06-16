import subprocess
import time
import socket
import os
from pathlib import Path


class ServerManager:
    """
    Handles the automated lifecycle of the Zappy C/Rust server process.
    Manages port allocation, execution arguments, and safe termination.
    """

    def __init__(
        self,
        port=8080,
        width=10,
        height=10,
        teams=["TeamAI", "TeamAI2", "TeamAI3", "TeamAI4"],
        binary_path=None,
    ):
        """
        Initializes the server configuration.
        Automatically calculates the absolute path to the server binary if none is provided.

        :param port: The starting port number to bind the server to.
        :param width: Map width.
        :param height: Map height.
        :param teams: List of team names allowed to connect.
        :param binary_path: Optional explicit path to the zappy_server binary.
        """
        self.port = port
        self.width = width
        self.height = height
        self.teams = teams
        self.process = None

        if binary_path:
            self.binary_path = binary_path
        else:
            self.binary_path = self._resolve_binary_path()

    def _resolve_binary_path(self):
        if env_path := os.getenv("ZAPPY_SERVER_PATH"):
            return env_path

        base_dir = Path(__file__).resolve().parents[3]

        possible_locations = [
            base_dir / "zappy_server",
            base_dir / "cmake-build-debug" / "zappy_server",
            base_dir / "server" / "zappy_server",
        ]

        for path in possible_locations:
            if path.is_file() and os.access(path, os.X_OK):
                return str(path)

        raise FileNotFoundError(
            "zappy_server executable not found. Please compile the server or set the ZAPPY_SERVER_PATH environment variable."
        )

    def get_free_port(self, current_port):
        """
        Finds and returns an available network port on the host machine.
        If the current_port is busy, asks the OS to provide a random free port.

        :param current_port: The preferred port number.
        :return: An available integer port number.
        """
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind(("", current_port))
                return current_port
            except OSError:
                s.bind(("", 0))
                return s.getsockname()[1]

    def start(self):
        """
        Spawns the Zappy server as a background subprocess with the required arguments.
        Includes a small sleep delay to ensure the server is ready to accept socket connections.
        """
        self.port = self.get_free_port(self.port)
        # -p 8080 -x 100 -y 100 -n team1 team2 team3 team4 -c 2 -f 10
        cmd = [
            self.binary_path,
            "-p",
            str(self.port),
            "-x",
            str(self.width),
            "-y",
            str(self.height),
            "-c",
            "2",
            "-f",
            "10",
            "-n",
        ] + self.teams

        self.process = subprocess.Popen(
            cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        time.sleep(0.2)

    def stop(self):
        """
        Safely terminates the server subprocess and waits for it to close,
        preventing zombie processes and hanging ports.
        """
        if self.process:
            self.process.terminate()
            self.process.wait()
            self.process = None
