#!/usr/bin/env python3

import sys
import socket
import argparse

class ZappyAiClient:
    def __init__(self, port, name, ip):
        self.port = port
        self.name = name
        self.ip = ip
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.buffer = ""

    def connect(self):
        """
        Connects to the server and performs the Zappy handshake.
        :return: 0 or 84 on error
        """
        try:
            self.socket.connect((self.ip, self.port))
            
            # receive welcom
            welcome = self.receive_line()
            if welcome != "WELCOME":
                print(f"Unexpected welcome message: {welcome}")
                return 84
            
            # send team name
            self.send_line(self.name)
            
            # receive slots
            slots = self.receive_line()
            print(f"Remaining slots: {slots}")
            
            # receive map dimensions
            dimensions = self.receive_line()
            print(f"Map dimensions: {dimensions}")
            
            return 0
        except (ConnectionRefusedError, socket.gaierror):
            print(f"Could not connect to server at {self.ip}:{self.port}")
            return 84
        except Exception as e:
            print(f"An error occurred during connection: {e}")
            return 84

    def send_line(self, message):
        """
        Sends a message to the server. Followed by a new line
        :param message: message to send
        :return:
        """
        self.socket.sendall((message + "\n").encode())

    def receive_line(self):
        """
        Reads from the socket until a newline is found and returns the line.
        :return: the line from the server without the newline character
        """
        while "\n" not in self.buffer:
            data = self.socket.recv(1024).decode()
            if not data:
                return None
            self.buffer += data
        
        line, self.buffer = self.buffer.split("\n", 1)
        return line.strip()

    def run(self):
        """
        Loop to handle server communication
        :return:
        """
        try:
            while True:
                line = self.receive_line()
                if line is None:
                    print("Server closed the connection.")
                    break
                if line == "dead":
                    print("You died.")
                    break
                print(f"Received: {line}")
                # here we will have the logic of using the AI
        except KeyboardInterrupt:
            pass
        finally:
            self.socket.close()

def main():
    """
    Main function. Handles parameter parsing and start the client.
    :return: 0 or 84 on error
    """
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-p", type=int, dest="port", help="port number")
    parser.add_argument("-n", type=str, dest="name", help="name of the team")
    parser.add_argument("-ip", type=str, dest="ip", help="ip address")
    parser.add_argument("-h", "--help", action="help", help="show help message")

    try:
        args = parser.parse_args()
        if args.port is None or args.name is None or args.ip is None:
            print("USAGE: ./zappy_ai -p port -n name -ip ip address")
            return 84
        
        client = ZappyAiClient(args.port, args.name, args.ip)
        if client.connect() == 0:
            client.run()
            return 0
        return 84
    except SystemExit as e:
        return 0 if e.code == 0 else 84
    except Exception:
        return 84

if __name__ == "__main__":
    sys.exit(main())
