import sys
import socket
import argparse


def connect(port, name, ip):
    """
    Connects to the Zappy serve using the port, name and ip address
    :param port: port number to connect to
    :param name: name of the team
    :param ip: ip address
    :return: 0 or 84 on error
    """
    s = socket.socket()

    try:
        s.connect((ip, port))
    except (ConnectionRefusedError, socket.gaierror):
        print(f"Could not connect to server at {ip}:{port}")
        return 84

    try:
        print(s.recv(1024).decode())
    except Exception:
        return 84
    finally:
        s.close()
    return 0


def main():
    """
    Main function, reads the command line arguments and calls the connect function
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
        return connect(args.port, args.name, args.ip)
    except SystemExit as e:
        return 0 if e.code == 0 else 84
    except Exception:
        return 84


if __name__ == "__main__":
    sys.exit(main())
