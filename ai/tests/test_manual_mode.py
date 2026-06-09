import unittest
import sys
import os
from unittest.mock import MagicMock, patch

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.strategy.manual_mode import run_manual
from src.client import ZappyAiClient


class TestManualMode(unittest.TestCase):
    def setUp(self):
        # Patching Connection to avoid real network calls
        self.patcher = patch("src.client.ai_client.Connection")
        self.mock_connection_class = self.patcher.start()
        self.mock_connection = self.mock_connection_class.return_value

        # Default response for commands
        self.mock_connection.receive_line.return_value = "ok"

        self.client = ZappyAiClient(4242, "team1", "127.0.0.1")
        self.client.forward = MagicMock(return_value="ok")
        self.client.right = MagicMock(return_value="ok")
        self.client.broadcast = MagicMock(return_value="ok")
        self.client.take = MagicMock(return_value="ok")
        self.client.close = MagicMock()

    def tearDown(self):
        self.patcher.stop()

    @patch("builtins.input")
    @patch("builtins.print")
    @patch("src.strategy.manual_mode.logger")
    def test_run_manual_commands(self, mock_logger, mock_print, mock_input):
        # We simulate user inputs:
        # 1. '1' (Forward)
        # 2. 'right' (Right command case-insensitive)
        # 3. '6 message' (Broadcast message)
        # 4. '10 food' (Take food)
        # 5. 'exit' (Exit manual mode)
        mock_input.side_effect = ["1", "right", "6 message", "10 food", "exit"]

        run_manual(self.client)

        self.client.forward.assert_called_once()
        self.client.right.assert_called_once()
        self.client.broadcast.assert_called_once_with("message")
        self.client.take.assert_called_once_with("food")
        self.client.close.assert_called_once()
        mock_logger.info.assert_any_call("Exiting manual mode...")

    @patch("builtins.input")
    @patch("builtins.print")
    @patch("src.strategy.manual_mode.logger")
    def test_run_manual_invalid_command(self, mock_logger, mock_print, mock_input):
        mock_input.side_effect = ["invalid_cmd", "exit"]

        run_manual(self.client)

        # Verify that we logged an error message
        mock_logger.error.assert_any_call(
            "Unknown command 'invalid_cmd'. Type 'help' to see available commands."
        )
        self.client.close.assert_called_once()


if __name__ == "__main__":
    unittest.main()
