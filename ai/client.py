import sys
import socket


def connect(port, name, ip):
    s = socket.socket()

    s.connect((ip, port))

    print(s.recv(1024).decode())

    s.close()


def print_help():
    print("USAGE: ./zappy_ai -p port -n name -ip ip address")


def main():
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
