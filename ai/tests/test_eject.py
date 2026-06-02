import unittest
import sys
import os
from unittest.mock import MagicMock, patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from src.client import ZappyAiClient

class TestEject(unittest.TestCase):

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

    def test_eject_success(self):
        """Test that eject sends the correct command and returns ok on success."""
        self.mock_connection.receive_line.return_value = "ok"
        response = self.client.eject()
        self.mock_connection.send_line.assert_called_with("Eject")
        self.assertEqual(response, "ok")

    def test_eject_failure(self):
        """Test that eject returns ko if server sends ko."""
        self.mock_connection.receive_line.return_value = "ko"
        response = self.client.eject()
        self.mock_connection.send_line.assert_called_with("Eject")
        self.assertEqual(response, "ko")

    def test_eject_death(self):
        """Test that eject returns 'dead' and sets is_dead to True if server sends 'dead'."""
        self.mock_connection.receive_line.return_value = "dead"
        response = self.client.eject()
        self.assertEqual(response, "dead")
        self.assertTrue(self.client.is_dead)

    def test_eject_with_broadcast(self):
        """Test that eject ignores broadcasts and returns 'ok' when it eventually arrives."""
        # Setup receive_line to return a message then 'ok'
        self.mock_connection.receive_line.side_effect = ["message 0, hello", "ok"]
        
        response = self.client.eject()
        
        self.assertEqual(response, "ok")
        self.assertEqual(len(self.client.messages), 1)
        self.assertEqual(self.client.messages[0], {"direction": 0, "text": "hello"})

if __name__ == '__main__':
    unittest.main()
