import unittest
import sys
import os
from unittest.mock import patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.client import ZappyAiClient


class TestIncantation(unittest.TestCase):
    def setUp(self):
        # Patching Connection to avoid real network calls
        self.patcher = patch("src.client.ai_client.Connection")
        self.mock_connection_class = self.patcher.start()
        self.mock_connection = self.mock_connection_class.return_value

        self.client = ZappyAiClient(4242, "team1", "127.0.0.1")

    def tearDown(self):
        self.patcher.stop()

    def test_incantation_success(self):
        # Simulate a successful incantation response sequence
        self.mock_connection.receive_line.side_effect = [
            "Elevation underway",
            "Current level: 2",
        ]

        result = self.client.incantation()

        # Verify the command sent
        self.mock_connection.send_line.assert_called_with("Incantation")

        # Updated behavior: returns the final result immediately and updates the level
        self.assertEqual(result, "Current level: 2")
        self.assertEqual(self.client.level, 2)

    def test_incantation_failure(self):
        # Simulate a failed incantation response (immediate failure)
        self.mock_connection.receive_line.return_value = "ko"

        result = self.client.incantation()

        # Verify the command sent
        self.mock_connection.send_line.assert_called_with("Incantation")

        # Verify the result and that level hasn't changed (default is 1)
        self.assertEqual(result, "ko")
        self.assertEqual(self.client.level, 1)

    def test_incantation_interrupted(self):
        # Simulate "Elevation underway" followed by "ko"
        self.mock_connection.receive_line.side_effect = ["Elevation underway", "ko"]

        result = self.client.incantation()

        # Updated behavior: waits for the final "ko" and returns it
        self.assertEqual(result, "ko")
        self.assertEqual(self.client.level, 1)


if __name__ == "__main__":
    unittest.main()
