import unittest
from unittest.mock import patch, MagicMock
import io
import sys
import socket
from client import ZappyAiClient, main

class TestClient(unittest.TestCase):

    @patch('socket.socket')
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_connect_success(self, mock_stdout, mock_socket):
        # Setup mock
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance
        
        # Sequence of responses for handshake: WELCOME, slots, dimensions
        mock_socket_instance.recv.side_effect = [
            b"WELCOME\n",
            b"10\n",
            b"20 20\n",
            b"" # End of data for any subsequent reads
        ]

        client = ZappyAiClient(4242, "team1", "127.0.0.1")
        result = client.connect()

        # Assertions
        self.assertEqual(result, 0)
        mock_socket_instance.connect.assert_called_with(("127.0.0.1", 4242))
        self.assertIn("Remaining slots: 10", mock_stdout.getvalue())
        self.assertIn("Map dimensions: 20 20", mock_stdout.getvalue())
        
        # Verify team name was sent
        mock_socket_instance.sendall.assert_any_call(b"team1\n")

    @patch('socket.socket')
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_connect_refused(self, mock_stdout, mock_socket):
        # Setup mock to raise ConnectionRefusedError
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance
        mock_socket_instance.connect.side_effect = ConnectionRefusedError

        client = ZappyAiClient(4242, "team1", "127.0.0.1")
        result = client.connect()

        # Assertions
        self.assertEqual(result, 84)
        self.assertIn("Could not connect to server at 127.0.0.1:4242", mock_stdout.getvalue())

    @patch('sys.argv', ['client.py', '-h'])
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_main_help(self, mock_stdout):
        result = main()
        self.assertEqual(result, 0)

    @patch('sys.argv', ['client.py'])
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_main_no_args(self, mock_stdout):
        result = main()
        self.assertEqual(result, 84)
        self.assertIn("USAGE: ./zappy_ai -p port -n name -ip ip address", mock_stdout.getvalue())

    @patch('sys.argv', ['client.py', '-p', '4242', '-n', 'team1', '-ip', '127.0.0.1'])
    @patch('client.ZappyAiClient')
    def test_main_valid_args(self, mock_zappy_client):
        # Setup mock client
        mock_instance = mock_zappy_client.return_value
        mock_instance.connect.return_value = 0
        
        # Mock run to avoid infinite loop
        mock_instance.run.return_value = None
        
        result = main()
        
        self.assertEqual(result, 0)
        mock_zappy_client.assert_called_with(4242, 'team1', '127.0.0.1')
        mock_instance.connect.assert_called_once()
        mock_instance.run.assert_called_once()

    def test_receive_line_buffered(self):
        # Test the buffered reading logic specifically
        mock_socket = MagicMock()
        client = ZappyAiClient(4242, "team1", "127.0.0.1")
        client.socket = mock_socket
        
        # Scenario: Multiple lines in one recv, then partial line
        mock_socket.recv.side_effect = [
            b"line1\nline2\npart",
            b"ial\n"
        ]
        
        self.assertEqual(client.receive_line(), "line1")
        self.assertEqual(client.receive_line(), "line2")
        self.assertEqual(client.receive_line(), "partial")

if __name__ == '__main__':
    unittest.main()
