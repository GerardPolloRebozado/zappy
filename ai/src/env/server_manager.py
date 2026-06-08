import subprocess
import time
import socket


class ServerManager:
    def __init__(
        self,
        port=4242,
        width=10,
        height=10,
        teams=["TeamAI"],
        binary_path="./zappy_server",
    ):
        self.port = port
        self.width = width
        self.height = height
        self.teams = teams
        self.binary_path = binary_path
        self.process = None

    def get_free_port(self, current_port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind(("", current_port))
                return current_port
            except OSError:
                s.bind(("", 0))
                return s.getsockname()[1]

    def start(self):
        self.port = self.get_free_port(self.port)

        cmd = [
            self.binary_path,
            "-p",
            str(self.port),
            "-x",
            str(self.width),
            "-y",
            str(self.height),
            "-n",
        ] + self.teams

        self.process = subprocess.Popen(
            cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        time.sleep(0.2)

    def stop(self):
        if self.process:
            self.process.terminate()
            self.process.wait()
            self.process = None
