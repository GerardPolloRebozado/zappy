import socket

class Connection:
    def __init__(self, ip, port):
        """
        Creates a connection to the server.
        :param ip: ip address
        :param port: port number
        """
        self.ip = ip
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.buffer = ""

    def connect(self):
        self.socket.connect((self.ip, self.port))

    def send_line(self, message):
        """
        Sends a message to the server followed by a newline.
        """
        self.socket.sendall((message + "\n").encode())

    def receive_line(self):
        """
        Reads from the socket until a newline is found and returns the line.
        """
        while "\n" not in self.buffer:
            data = self.socket.recv(1024).decode()
            if not data:
                return None
            self.buffer += data
        
        line, self.buffer = self.buffer.split("\n", 1)
        return line.strip()

    def close(self):
        self.socket.close()
