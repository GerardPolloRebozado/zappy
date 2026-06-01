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
