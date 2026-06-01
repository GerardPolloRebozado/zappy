import socket
from src.network import Connection
from src.utils import parse_look, Inventory
from . import commands

class ZappyAiClient:
    def __init__(self, port, name, ip):
        """
        Initializes a ZappyAiClient object.
        :param port: port number
        :param name: team name
        :param ip: ip address
        """
        self.port = port
        self.name = name
        self.ip = ip
        self.connection = Connection(ip, port)
        self.messages = []
        self.is_dead = False

    def connect(self):
        """
        Connects to the server and performs the Zappy handshake.
        :return: 0 or 84 on error
        """
        try:
            self.connection.connect()
            
            # receive welcome
            welcome = self.connection.receive_line()
            if welcome != "WELCOME":
                print(f"Unexpected welcome message: {welcome}")
                return 84
            
            # send team name
            self.connection.send_line(self.name)
            
            # receive slots
            slots = self.connection.receive_line()
            print(f"Remaining slots: {slots}")
            
            # receive map dimensions
            dimensions = self.connection.receive_line()
            print(f"Map dimensions: {dimensions}")
            
            return 0
        except (ConnectionRefusedError, socket.gaierror):
            print(f"Could not connect to server at {self.ip}:{self.port}")
            return 84
        except Exception as e:
            print(f"An error occurred during connection: {e}")
            return 84

    def receive_line(self):
        return self.connection.receive_line()

    def wait_for_response(self):
        """
        Wait for a response while catching broadcasts and death.
        Returns:
            - The response string (ok, [tile1...], etc.)
            - None if the connection is closed.
        """
        while True:
            line = self.connection.receive_line()
            match line:
                case None:
                    return None
                case "":
                    continue
                case "dead":
                    self.is_dead = True
                    return "dead"
                case s if s.startswith("message"):
                    self.messages.append(s)
                    continue
                case _:
                    return line

    def forward(self):
        commands.forward(self)
        return self.wait_for_response()

    def right(self):
        commands.right(self)
        return self.wait_for_response()

    def left(self):
        commands.left(self)
        return self.wait_for_response()

    def look(self):
        commands.look(self)
        resp = self.wait_for_response()
        return parse_look(resp) if resp and resp.startswith("[") else resp

    def inventory(self):
        commands.inventory(self)
        resp = self.wait_for_response()
        return Inventory.from_string(resp) if resp and resp.startswith("[") else resp

    def close(self):
        self.connection.close()
