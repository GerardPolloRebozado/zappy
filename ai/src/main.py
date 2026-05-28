#!/usr/bin/env python3

import sys
import os
import argparse

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')))

from src.client.ai_client import ZappyAiClient


def run_client(client):
    """
    Loop to handle server communication
    :return:
    """
    try:
        while True:
            line = client.receive_line()
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
        client.close()

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
            run_client(client)
            return 0
        return 84
    except SystemExit as e:
        return 0 if e.code == 0 else 84
    except Exception:
        return 84

if __name__ == "__main__":
    sys.exit(main())
