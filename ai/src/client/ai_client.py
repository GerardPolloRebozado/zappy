import socket
from src.network import Connection
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

    def forward(self):
        commands.forward(self)

    def right(self):
        commands.right(self)

    def left(self):
        commands.left(self)

    def close(self):
        self.connection.close()
