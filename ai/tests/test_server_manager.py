import unittest
import os
from training.env.server_manager import ServerManager


class TestServerManager(unittest.TestCase):
    def test_server_lifecycle(self):
        print("\n[Test Server] Starting server")

        binary_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../../server/zappy_server")
        )

        manager = ServerManager(binary_path=binary_path)

        manager.start()
        self.assertIsNotNone(manager.process, "The server porcess should not be None")

        print("Killing Server")
        manager.stop()
        self.assertIsNone(manager.process, "The server porcess should be None")
        print("[Test Server] END")


if __name__ == "__main__":
    unittest.main()
