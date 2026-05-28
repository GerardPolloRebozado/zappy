import unittest
from unittest.mock import patch, MagicMock
import io
import sys
import socket
from client import connect, main

class TestClient(unittest.TestCase):

    @patch('socket.socket')
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_connect_success(self, mock_stdout, mock_socket):
        # Setup mock
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance
        mock_socket_instance.recv.return_value = b"Welcome to Zappy!"

        # Call function
        result = connect(4242, "team1", "127.0.0.1")

        # Assertions
        self.assertEqual(result, 0)
        mock_socket_instance.connect.assert_called_with(("127.0.0.1", 4242))
        mock_socket_instance.recv.assert_called_with(1024)
        self.assertIn("Welcome to Zappy!", mock_stdout.getvalue())
        mock_socket_instance.close.assert_called_once()

    @patch('socket.socket')
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_connect_refused(self, mock_stdout, mock_socket):
        # Setup mock to raise ConnectionRefusedError
        mock_socket_instance = MagicMock()
        mock_socket.return_value = mock_socket_instance
        mock_socket_instance.connect.side_effect = ConnectionRefusedError

        # Call function
        result = connect(4242, "team1", "127.0.0.1")

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
    @patch('client.connect')
    def test_main_valid_args(self, mock_connect):
        mock_connect.return_value = 0
        result = main()
        self.assertEqual(result, 0)
        mock_connect.assert_called_with(4242, 'team1', '127.0.0.1')

    @patch('sys.argv', ['client.py', '-p', '4242'])
    @patch('sys.stdout', new_callable=io.StringIO)
    def test_main_missing_args(self, mock_stdout):
        result = main()
        self.assertEqual(result, 84)
        self.assertIn("USAGE: ./zappy_ai -p port -n name -ip ip address", mock_stdout.getvalue())

    @patch('sys.argv', ['client.py', '-n', 'team1', '-p', '4242', '-ip', '127.0.0.1'])
    @patch('client.connect')
    def test_main_different_order(self, mock_connect):
        mock_connect.return_value = 0
        result = main()
        self.assertEqual(result, 0)
        mock_connect.assert_called_with(4242, 'team1', '127.0.0.1')

if __name__ == '__main__':
    unittest.main()
