import unittest
import sys
import os
from unittest.mock import patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.client import ZappyAiClient


class TestConnectNbr(unittest.TestCase):
    def setUp(self):
        # Patching Connection to avoid real network calls
        self.patcher = patch("src.client.ai_client.Connection")
        self.mock_connection_class = self.patcher.start()
        self.mock_connection = self.mock_connection_class.return_value

        self.client = ZappyAiClient(4242, "team1", "127.0.0.1")

    def tearDown(self):
        self.patcher.stop()

    def test_connect_nbr_success(self):
        # Server returns 10 unused slots
        self.mock_connection.receive_line.return_value = "10"

        result = self.client.connect_nbr()

        self.mock_connection.send_line.assert_called_with("Connect_nbr")
        self.assertEqual(result, 10)

    def test_connect_nbr_zero(self):
        # Server returns 0 unused slots
        self.mock_connection.receive_line.return_value = "0"

        result = self.client.connect_nbr()

        self.mock_connection.send_line.assert_called_with("Connect_nbr")
        self.assertEqual(result, 0)

    def test_connect_nbr_invalid(self):
        # Server returns an invalid response (unlikely but should be handled)
        self.mock_connection.receive_line.return_value = "invalid"

        result = self.client.connect_nbr()

        self.assertEqual(result, "invalid")

    def test_connect_nbr_none(self):
        # Server closes connection
        self.mock_connection.receive_line.return_value = None

        result = self.client.connect_nbr()

        self.assertIsNone(result)

    def test_connect_nbr_with_broadcast(self):
        # Server sends a broadcast message before the connect_nbr response
        self.mock_connection.receive_line.side_effect = ["message 0, text", "5"]

        result = self.client.connect_nbr()

        self.assertEqual(result, 5)
        self.assertEqual(len(self.client.messages), 1)
        self.assertEqual(self.client.messages[0], {"direction": 0, "text": "text"})

    def test_connect_nbr_death(self):
        # Server sends "dead" while waiting for response
        self.mock_connection.receive_line.return_value = "dead"

        result = self.client.connect_nbr()

        self.assertEqual(result, "dead")
        self.assertTrue(self.client.is_dead)


if __name__ == "__main__":
    unittest.main()
