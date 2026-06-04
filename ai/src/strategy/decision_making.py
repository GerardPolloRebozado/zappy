from src.client import ZappyAiClient
from src.strategy import run_client

from src.utils import inventory, can_evolve

def count_players_in_tile(tile):
    players = 0
    content = tile.split(",")
    for item in content:
        if item.startswith("Player"):
            players += 1
    return players

def decision_making():

    client = ZappyAiClient()
    inventory = client.inventory()
    look = client.look()
    tile = look[0]
    players = count_players_in_tile(tile)

    if can_evolve(client.level, inventory, players):
        client.incantation()

    return 0