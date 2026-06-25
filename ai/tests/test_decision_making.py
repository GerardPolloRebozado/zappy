import unittest
from unittest.mock import MagicMock, patch
from src.strategy.decision_making import (
    get_missing_resources,
    move_to_tile,
    count_players_in_tile,
    take_decision,
)
from src.utils.inventory import Inventory


class TestDecisionMaking(unittest.TestCase):
    def test_get_missing_resources_level_1_empty(self):
        inv = Inventory()
        missing = get_missing_resources(1, inv)
        self.assertIn("linemate", missing)
        self.assertIn("food", missing)  # food < 10

    def test_get_missing_resources_level_1_full(self):
        inv = Inventory(food=25, linemate=1)
        missing = get_missing_resources(1, inv)
        self.assertEqual(missing, [])

    def test_get_missing_resources_invalid_level(self):
        inv = Inventory()
        missing = get_missing_resources(99, inv)
        self.assertEqual(missing, [])

    def test_move_to_tile_0(self):
        client = MagicMock()
        self.assertTrue(move_to_tile(client, 0))
        client.forward.assert_not_called()

    def test_move_to_tile_2(self):
        client = MagicMock()
        move_to_tile(client, 2)
        client.forward.assert_called_once()
        client.left.assert_not_called()
        client.right.assert_not_called()

    def test_move_to_tile_8(self):
        client = MagicMock()
        move_to_tile(client, 8)
        self.assertEqual(
            client.forward.call_count, 4
        )  # 2 forward, then turn, then 2 forward
        client.right.assert_called_once()

    def test_move_to_tile_4(self):
        client = MagicMock()
        move_to_tile(client, 4)
        self.assertEqual(client.forward.call_count, 4)
        client.left.assert_called_once()

    def test_count_players_in_tile(self):
        self.assertEqual(count_players_in_tile(["player", "food", "player"]), 2)
        self.assertEqual(count_players_in_tile(["food", "linemate"]), 0)

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_can_evolve(self, mock_can_evolve):
        client = MagicMock()
        client.level = 1
        client.inventory.return_value = Inventory(food=25, linemate=1)
        client.look.return_value = [["player", "food"], ["linemate"]]
        mock_can_evolve.return_value = True

        take_decision(client)

        client.set.assert_called_with("linemate")
        client.incantation.assert_called_once()

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_waiting_for_players(self, mock_can_evolve):
        client = MagicMock()
        client.name = "TeamA"
        client.level = 2  # Requires 2 players
        client.inventory.return_value = Inventory(
            food=25, linemate=1, deraumere=1, sibur=1
        )
        client.look.return_value = [["player"], ["food"]]
        mock_can_evolve.return_value = False

        take_decision(client)

        client.broadcast.assert_called_with("Elevation TeamA level 2")

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_take_resource_on_tile(self, mock_can_evolve):
        client = MagicMock()
        client.level = 1
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [["player", "linemate"], ["food"]]
        mock_can_evolve.return_value = False

        take_decision(client)

        client.take.assert_called_with("linemate")

    @patch("src.strategy.decision_making.can_evolve")
    @patch("src.strategy.decision_making.move_to_tile")
    def test_take_decision_move_to_resource(self, mock_move_to_tile, mock_can_evolve):
        client = MagicMock()
        client.level = 1
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [["player"], ["linemate"]]  # linemate on tile 1
        mock_can_evolve.return_value = False

        take_decision(client)

        mock_move_to_tile.assert_called_with(client, 1)

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_explore(self, mock_can_evolve):
        client = MagicMock()
        client.level = 1
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [
            ["player"],
            ["food"],
            ["food"],
        ]  # No linemate in sight
        mock_can_evolve.return_value = False

        take_decision(client)

        client.forward.assert_called_once()

    def test_take_decision_inventory_fail(self):
        client = MagicMock()
        client.inventory.return_value = None
        take_decision(client)
        client.look.assert_not_called()

    def test_take_decision_look_fail(self):
        client = MagicMock()
        client.inventory.return_value = Inventory()
        client.look.return_value = "ko"
        take_decision(client)
        client.forward.assert_not_called()

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_same_tile_broadcast(self, mock_can_evolve):
        client = MagicMock()
        client.name = "TeamA"
        client.level = 2
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [["player"], ["food"]]
        client.messages = [{"direction": 0, "text": "Elevation TeamA level 2"}]
        mock_can_evolve.return_value = False

        take_decision(client)

        client.inventory.assert_called()
        self.assertEqual(len(client.messages), 0)

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_steering_broadcast(self, mock_can_evolve):
        client = MagicMock()
        client.name = "TeamA"
        client.level = 2
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [["player"], ["food"]]
        client.messages = [{"direction": 3, "text": "Elevation TeamA level 2"}]
        mock_can_evolve.return_value = False

        take_decision(client)

        client.left.assert_called_once()
        self.assertEqual(len(client.messages), 0)

    @patch("src.strategy.decision_making.can_evolve")
    def test_take_decision_ignore_other_broadcasts(self, mock_can_evolve):
        client = MagicMock()
        client.name = "TeamA"
        client.level = 2
        client.inventory.return_value = Inventory(food=25, linemate=0)
        client.look.return_value = [["player"], ["food"]]
        # Message for TeamB or different level should be ignored
        client.messages = [
            {"direction": 3, "text": "Elevation TeamB level 2"},
            {"direction": 3, "text": "Elevation TeamA level 3"},
        ]
        mock_can_evolve.return_value = False

        take_decision(client)

        # Should fall through to searching for resource (food not missing since food=25, linemate missing, not in sight, so explores)
        client.forward.assert_called_once()
        # Messages should NOT be cleared if they are ignored (or wait, the messages remain in the queue or we don't clear them in that case? Yes, they remain or they are not acted upon).
        # Actually in the code we only clear messages when target_msg is found and handled. So here they shouldn't be cleared:
        self.assertNotEqual(len(client.messages), 0)


if __name__ == "__main__":
    unittest.main()
