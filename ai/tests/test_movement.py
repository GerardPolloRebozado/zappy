import unittest
import sys
import os
from unittest.mock import MagicMock, patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from src.client import ZappyAiClient

class TestMovement(unittest.TestCase):

    def setUp(self):
        # Patching Connection to avoid real network calls
        self.patcher = patch('src.client.ai_client.Connection')
        self.mock_connection_class = self.patcher.start()
        self.mock_connection = self.mock_connection_class.return_value
        
        # Default response for commands
        self.mock_connection.receive_line.return_value = "ok"
        
        self.client = ZappyAiClient(4242, "team1", "127.0.0.1")

    def tearDown(self):
        self.patcher.stop()

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
