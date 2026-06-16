import unittest
import sys
import os
from unittest.mock import patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.client import ZappyAiClient


class TestTakeSet(unittest.TestCase):
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

    def test_take_success(self):
        """Test that take sends the correct command and returns ok on success."""
        self.mock_connection.receive_line.return_value = "ok"
        response = self.client.take("food")
        self.mock_connection.send_line.assert_called_with("Take food")
        self.assertEqual(response, "ok")

    def test_take_failure(self):
        """Test that take returns ko if server sends ko (object not present)."""
        self.mock_connection.receive_line.return_value = "ko"
        response = self.client.take("linemate")
        self.mock_connection.send_line.assert_called_with("Take linemate")
        self.assertEqual(response, "ko")

    def test_set_success(self):
        """Test that set sends the correct command and returns ok on success."""
        self.mock_connection.receive_line.return_value = "ok"
        response = self.client.set("food")
        self.mock_connection.send_line.assert_called_with("Set food")
        self.assertEqual(response, "ok")

    def test_set_failure(self):
        """Test that set returns ko if server sends ko (object not in inventory)."""
        self.mock_connection.receive_line.return_value = "ko"
        response = self.client.set("linemate")
        self.mock_connection.send_line.assert_called_with("Set linemate")
        self.assertEqual(response, "ko")

    def test_take_death(self):
        """Test that take returns 'dead' and sets is_dead to True if server sends 'dead'."""
        self.mock_connection.receive_line.return_value = "dead"
        response = self.client.take("food")
        self.assertEqual(response, "dead")
        self.assertTrue(self.client.is_dead)

    def test_set_with_broadcast(self):
        """Test that set ignores broadcasts and returns 'ok' when it eventually arrives."""
        # Setup receive_line to return a message then 'ok'
        self.mock_connection.receive_line.side_effect = ["message 0, help", "ok"]

        response = self.client.set("food")

        self.assertEqual(response, "ok")
        self.assertEqual(len(self.client.messages), 1)
        self.assertEqual(self.client.messages[0], {"direction": 0, "text": "help"})


if __name__ == "__main__":
    unittest.main()
