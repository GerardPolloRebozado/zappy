import socket
from src.network import Connection
from src.utils import parse_look, parse_broadcast, Inventory
from . import commands
from src.utils.logging_levels import logger


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
        self.level = 1

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
                logger.warning(f"Unexpected welcome message: {welcome}")
                return 84

            # send team name
            self.connection.send_line(self.name)

            # receive slots
            slots = self.connection.receive_line()
            logger.info(f"Remaining slots: {slots}")
            # receive map dimensions
            dimensions = self.connection.receive_line()
            logger.info(f"Map dimensions: {dimensions}")
            return 0
        except (ConnectionRefusedError, socket.gaierror):
            logger.error(f"Could not connect to server at {self.ip}:{self.port}")
            return 84
        except Exception as e:
            logger.error(f"An error occurred during connection: {e}")
            return 84

    def receive_line(self):
        """
        Calls receive_line to receive a line from the server.
        :return: the line
        """
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
                    parsed = parse_broadcast(s)
                    if parsed:
                        self.messages.append(parsed)
                    continue
                case s if s.startswith("eject:"):
                    logger.info(f"Event: {s}")
                    continue
                case s if s.startswith("Current level:"):
                    try:
                        self.level = int(s.split(":")[1].strip())
                        logger.info(f"Event: {s}")
                    except (ValueError, IndexError):
                        logger.warning(f"Failed to parse level up message: {s}")
                    return s
                case _:
                    return line

    def forward(self):
        """
        Calls movement command forward
        :return: ok
        """
        commands.forward(self)
        return self.wait_for_response()

    def right(self):
        """
        Calls movement command right
        :return: ok
        """
        commands.right(self)
        return self.wait_for_response()

    def left(self):
        """
        Calls movement command left
        :return: ok
        """
        commands.left(self)
        return self.wait_for_response()

    def look(self):
        """
        Calls command look
        :return: The parsed tile information
        """
        commands.look(self)
        resp = self.wait_for_response()
        return parse_look(resp) if resp and resp.startswith("[") else resp

    def inventory(self):
        """
        Calls command inventory
        :return: The parsed inventory
        """
        commands.inventory(self)
        resp = self.wait_for_response()
        return Inventory.from_string(resp) if resp and resp.startswith("[") else resp

    def broadcast(self, text):
        """
        Calls command broadcast
        :param text: the message to broadcast
        :return: ok
        """
        commands.broadcast(self, text)
        return self.wait_for_response()

    def connect_nbr(self):
        """
        Calls command connect_nbr
        :return: The number of free slots
        """
        commands.connect_nbr(self)
        resp = self.wait_for_response()
        try:
            return int(resp) if resp else None
        except ValueError:
            return resp

    def fork(self):
        """
        Calls command fork
        :return: ok
        """
        commands.fork(self)
        return self.wait_for_response()

    def eject(self):
        """
        Calls command eject
        :return: ok/ko
        """
        commands.eject(self)
        return self.wait_for_response()

    def take(self, object):
        """
        Calls command take
        :param object: Object to take
        :return: ok/ko
        """
        commands.take(self, object)
        return self.wait_for_response()

    def set(self, object):
        """
        Calls command set
        :param object: Object to set
        :return: ok/ko
        """
        commands.set(self, object)
        return self.wait_for_response()

    def incantation(self):
        """
        Calls command incantation, waits for completion if it starts.
        :return: Current level: X / ko
        """
        commands.incantation(self)
        first_resp = self.wait_for_response()
        if first_resp == "Elevation underway":
            final_resp = self.wait_for_response()
            return final_resp
        return first_resp

    def close(self):
        self.connection.close()
