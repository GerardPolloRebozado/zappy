def forward(client):
    """
    Move the player up one tile.
    Time limit: 7/f
    Response: ok
    """
    client.connection.send_line("Forward")


def right(client):
    """
    Turn the player 90 degrees right.
    Time limit: 7/f
    Response: ok
    """
    client.connection.send_line("Right")


def left(client):
    """
    Turn the player 90 degrees left.
    Time limit: 7/f
    Response: ok
    """
    client.connection.send_line("Left")


def look(client):
    """
    Look at the tiles in front of the player.
    Time limit: 7/f
    Response: [tile1, tile2, ...]
    """
    client.connection.send_line("Look")


def inventory(client):
    """
    Get the player's inventory.
    Time limit: 1/f
    Response: [food x, linemate y, ...]
    """
    client.connection.send_line("Inventory")


def broadcast(client, text):
    """
    Broadcast a message to all players.
    Time limit: 7/f
    Response: ok
    """
    client.connection.send_line(f"Broadcast {text}")


def connect_nbr(client):
    """
    Get the number of unused team slots.
    Time limit: -
    Response: value
    """
    client.connection.send_line("Connect_nbr")


def fork(client):
    """
    Forks a player
    Time limit: 42/f
    Response: ok
    """
    client.connection.send_line("Fork")


def eject(client):
    """
    Eject players from this tile
    Time limit: 7/f
    Response: ok/ko
    """
    client.connection.send_line("Eject")


def take(client, object):
    """
    Take an object from the tile.
    Time limit: 7/f
    Response: ok/ko
    """
    client.connection.send_line(f"Take {object}")


def set(client, object):
    """
    Sets an object on the tile.
    Time limit: 7/f
    Response: ok/ko
    """
    client.connection.send_line(f"Set {object}")


def incantation(client):
    """
    Start incantation
    Time limit: 300/f
    Response: Elevation underway/Current level: k/ko
    """
    client.connection.send_line("Incantation")
