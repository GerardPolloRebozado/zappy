import subprocess
import time
import socket
import os


class ServerManager:
    """
    Handles the automated lifecycle of the Zappy C/Rust server process.
    Manages port allocation, execution arguments, and safe termination.
    """

    def __init__(
        self,
        port=4242,
        width=10,
        height=10,
        teams=["TeamAI"],
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

        if binary_path is None:
            base_directory = os.path.dirname(os.path.abspath(__file__))
            self.binary_path = os.path.abspath(
                os.path.join(base_directory, "../../../server/zappy_server")
            )
        else:
            self.binary_path = binary_path

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

        cmd = [
            self.binary_path,
            "-p",
            str(self.port),
            "-x",
            str(self.width),
            "-y",
            str(self.height),
            "-c",
            "10",
            "-f",
            "100",
            "-n",
        ] + self.teams

        # stdout and stderr are suppressed to avoid polluting the AI training logs
        self.process = subprocess.Popen(cmd, stdout=None, stderr=None)
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
