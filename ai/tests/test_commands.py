import unittest
import sys
import os
from unittest.mock import MagicMock, patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from src.client import ZappyAiClient

class TestCommands(unittest.TestCase):

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

    def test_look(self):
        # Simulate a response from the server
        self.mock_connection.receive_line.return_value = "[player, food, , linemate]"
        
        result = self.client.look()
        
        # Verify the command sent
        self.mock_connection.send_line.assert_called_with("Look")
        
        # Verify the parsing logic
        expected = [['player'], ['food'], [], ['linemate']]
        self.assertEqual(result, expected)

    def test_inventory(self):
        # Simulate a response from the server
        self.mock_connection.receive_line.return_value = "[food 10, linemate 5, deraumere 0, sibur 1, mendiane 2, phiras 3, thystame 4]"
        
        result = self.client.inventory()
        
        # Verify the command sent
        self.mock_connection.send_line.assert_called_with("Inventory")
        
        # Verify the parsing logic
        self.assertEqual(result.food, 10)
        self.assertEqual(result.linemate, 5)
        self.assertEqual(result.deraumere, 0)
        self.assertEqual(result.sibur, 1)
        self.assertEqual(result.mendiane, 2)
        self.assertEqual(result.phiras, 3)
        self.assertEqual(result.thystame, 4)

    def test_inventory_partial(self):
        # Simulate a partial response from the server
        self.mock_connection.receive_line.return_value = "[food 5, sibur 2]"
        
        result = self.client.inventory()
        
        # Verify the command sent
        self.mock_connection.send_line.assert_called_with("Inventory")
        
        # Verify the parsing logic defaults missing items to 0
        self.assertEqual(result.food, 5)
        self.assertEqual(result.sibur, 2)
        self.assertEqual(result.linemate, 0)
        self.assertEqual(result.deraumere, 0)
        self.assertEqual(result.mendiane, 0)
        self.assertEqual(result.phiras, 0)
        self.assertEqual(result.thystame, 0)

if __name__ == '__main__':
    unittest.main()
