import unittest
import sys
import os
from unittest.mock import patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.client import ZappyAiClient


class TestFork(unittest.TestCase):
    def setUp(self):
        # Patching Connection to avoid real network calls
        self.patcher = patch("src.client.ai_client.Connection")
        self.mock_connection_class = self.patcher.start()
        self.mock_connection = self.mock_connection_class.return_value

        # Default response for commands
        self.mock_connection.receive_line.return_value = "ok"

        self.client = ZappyAiClient(4242, "team1", "127.0.0.1")

    def tearDown(self):
        self.patcher.stop()

    def test_fork_success(self):
        """Test that fork sends the correct command and returns ok on success."""
        self.mock_connection.receive_line.return_value = "ok"
        response = self.client.fork()
        self.mock_connection.send_line.assert_called_with("Fork")
        self.assertEqual(response, "ok")

    def test_fork_death(self):
        """Test that fork returns 'dead' and sets is_dead to True if server sends 'dead'."""
        self.mock_connection.receive_line.return_value = "dead"
        response = self.client.fork()
        self.assertEqual(response, "dead")
        self.assertTrue(self.client.is_dead)

    def test_fork_with_broadcast(self):
        """Test that fork ignores broadcasts and returns 'ok' when it eventually arrives."""
        # Setup receive_line to return a message then 'ok'
        self.mock_connection.receive_line.side_effect = ["message 0, hello", "ok"]

        response = self.client.fork()

        self.assertEqual(response, "ok")
        self.assertEqual(len(self.client.messages), 1)
        self.assertEqual(self.client.messages[0], {"direction": 0, "text": "hello"})


if __name__ == "__main__":
    unittest.main()
