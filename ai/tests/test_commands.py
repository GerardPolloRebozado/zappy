import unittest
from unittest.mock import MagicMock, patch
from src.client import ZappyAiClient

class TestCommands(unittest.TestCase):

    def setUp(self):
        # Patching Connection to avoid real network calls
        with patch('src.client.ai_client.Connection') as mock_connection_class:
            self.mock_connection = mock_connection_class.return_value
            self.client = ZappyAiClient(4242, "team1", "127.0.0.1")

    def test_forward(self):
        self.client.forward()
        self.mock_connection.send_line.assert_called_with("Forward")

    def test_right(self):
        self.client.right()
        self.mock_connection.send_line.assert_called_with("Right")

    def test_left(self):
        self.client.left()
        self.mock_connection.send_line.assert_called_with("Left")

if __name__ == '__main__':
    unittest.main()
