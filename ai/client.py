import sys
import socket


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
    except ConnectionRefusedError:
        print(f"Could not connect to server at {ip}:{port}")
        return 84

    print(s.recv(1024).decode())

    s.close()
    return 0


def print_help():
    """
    Prints help message
    :return: nothing
    """
    print("USAGE: ./zappy_ai -p port -n name -ip ip address")


def main():
    """
    Main function, reads the command line arguments and calls the connect function
    :return: 0 or 84 on error
    """
    params = sys.argv[1:]

    if params == []:
        print_help()
        return 84
    if params[0] == "-h" or params[0] == "--help":
        print_help()
        return 0

    if params[0] == "-p":
        port = int(params[1])
    if params[2] == "-n":
        name = str(params[3])
    if params[4] == "-ip":
        ip = str(params[5])
    connect(port, name, ip)
    return 0


if __name__ == "__main__":
    main()
