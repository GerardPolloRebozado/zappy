import socket
import selectors


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
        self.selector = selectors.DefaultSelector()
        self.buffer = ""

    def connect(self):
        """
        Connects to the server and sets the socket to non-blocking mode.
        """
        self.socket.connect((self.ip, self.port))
        self.socket.setblocking(False)

    def send_line(self, message):
        """
        Sends a message to the server followed by a newline.
        Uses selectors to handle non-blocking write.
        """
        msg = (message + "\n").encode()
        self.selector.register(self.socket, selectors.EVENT_WRITE)

        try:
            total_sent = 0
            while total_sent < len(msg):
                events = self.selector.select()
                for key, mask in events:
                    if mask & selectors.EVENT_WRITE:
                        sent = self.socket.send(msg[total_sent:])
                        if sent == 0:
                            raise RuntimeError("Socket connection broken")
                        total_sent += sent
        finally:
            try:
                self.selector.unregister(self.socket)
            except KeyError:
                pass

    def receive_line(self, timeout=None):
        """
        Reads from the socket until a newline is found and returns the line.
        Uses selectors to handle non-blocking read.
        """
        try:
            self.selector.register(self.socket, selectors.EVENT_READ)
        except (FileExistsError, KeyError):
            pass

        try:
            while "\n" not in self.buffer:
                events = self.selector.select(timeout)
                if not events:
                    return ""

                for key, mask in events:
                    if mask & selectors.EVENT_READ:
                        data = self.socket.recv(1024).decode()
                        if not data:
                            return None
                        self.buffer += data

            line, self.buffer = self.buffer.split("\n", 1)
            return line.strip()
        finally:
            try:
                self.selector.unregister(self.socket)
            except KeyError:
                pass

    def close(self):
        """
        Closes the socket and the selector.
        """
        self.selector.close()
        self.socket.close()
