import unittest
import sys
import os

# Add the project root to the python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from src.utils import parse_broadcast


class TestBroadcastParsing(unittest.TestCase):
    def test_parse_broadcast_success(self):
        data = "message 4, hello world"
        expected = {"direction": 4, "text": "hello world"}
        self.assertEqual(parse_broadcast(data), expected)

    def test_parse_broadcast_zero(self):
        data = "message 0, test"
        expected = {"direction": 0, "text": "test"}
        self.assertEqual(parse_broadcast(data), expected)

    def test_parse_broadcast_invalid_format(self):
        self.assertIsNone(parse_broadcast("invalid message"))
        self.assertIsNone(parse_broadcast("message K, text"))


if __name__ == "__main__":
    unittest.main()
