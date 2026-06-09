import unittest
import sys
import os
from unittest.mock import patch, MagicMock
import io
import selectors

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.client import ZappyAiClient
from src.network import Connection
from src.main import main


class TestClient(unittest.TestCase):
    def setUp(self):
        # Patch selectors to prevent hanging with mock sockets
        self.patcher = patch("selectors.DefaultSelector")
        self.mock_selector_class = self.patcher.start()
        self.mock_selector = self.mock_selector_class.return_value

        # Make select() return that the socket is ready
        self.mock_selector.select.return_value = [
            (MagicMock(), selectors.EVENT_READ | selectors.EVENT_WRITE)
        ]

    def tearDown(self):
        self.patcher.stop()

    @patch("src.network.connection.socket.socket")
    def test_connect_success(self, mock_socket):
        # Setup mock
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance

        # Mock send to return number of bytes "sent"
        mock_socket_instance.send.side_effect = lambda x: len(x)

        # Sequence of responses for handshake: WELCOME, slots, dimensions
        mock_socket_instance.recv.side_effect = [
            b"WELCOME\n",
            b"10\n",
            b"20 20\n",
            b"",  # End of data for any subsequent reads
        ]

        client = ZappyAiClient(4242, "team1", "127.0.0.1")
        with self.assertLogs("zappy_ai", level="INFO") as cm:
            result = client.connect()

        # Assertions
        self.assertEqual(result, 0)
        mock_socket_instance.connect.assert_called_with(("127.0.0.1", 4242))
        self.assertTrue(any("Remaining slots: 10" in output for output in cm.output))
        self.assertTrue(any("Map dimensions: 20 20" in output for output in cm.output))

        # Verify team name was sent
        mock_socket_instance.send.assert_any_call(b"team1\n")

    @patch("src.network.connection.socket.socket")
    def test_connect_refused(self, mock_socket):
        # Setup mock to raise ConnectionRefusedError
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance
        mock_socket_instance.connect.side_effect = ConnectionRefusedError

        client = ZappyAiClient(4242, "team1", "127.0.0.1")
        with self.assertLogs("zappy_ai", level="ERROR") as cm:
            result = client.connect()

        # Assertions
        self.assertEqual(result, 84)
        self.assertTrue(
            any(
                "Could not connect to server at 127.0.0.1:4242" in output
                for output in cm.output
            )
        )

    @patch("sys.argv", ["main.py", "-h"])
    @patch("sys.stdout", new_callable=io.StringIO)
    def test_main_help(self, mock_stdout):
        result = main()
        self.assertEqual(result, 0)

    @patch("sys.argv", ["main.py"])
    @patch("sys.stdout", new_callable=io.StringIO)
    def test_main_no_args(self, mock_stdout):
        result = main()
        self.assertEqual(result, 84)
        self.assertIn(
            "USAGE: ./zappy_ai -p port -n name -ip ip address", mock_stdout.getvalue()
        )

    @patch("sys.argv", ["main.py", "-p", "4242", "-n", "team1", "-ip", "127.0.0.1"])
    @patch("src.main.ZappyAiClient")
    @patch("src.main.run_client")
    def test_main_valid_args(self, mock_run_client, mock_zappy_client):
        # Setup mock client
        mock_instance = mock_zappy_client.return_value
        mock_instance.connect.return_value = 0

        result = main()

        self.assertEqual(result, 0)
        mock_zappy_client.assert_called_with(4242, "team1", "127.0.0.1")
        mock_instance.connect.assert_called_once()
        mock_run_client.assert_called_once()

    @patch("src.network.connection.socket.socket")
    def test_receive_line_buffered(self, mock_socket_class):
        # Test the buffered reading logic specifically
        mock_socket = MagicMock()
        conn = Connection("127.0.0.1", 4242)
        conn.socket = mock_socket

        # Scenario: Multiple lines in one recv, then partial line
        mock_socket.recv.side_effect = [b"line1\nline2\npart", b"ial\n"]

        self.assertEqual(conn.receive_line(), "line1")
        self.assertEqual(conn.receive_line(), "line2")
        self.assertEqual(conn.receive_line(), "partial")
        conn.close()


if __name__ == "__main__":
    unittest.main()
