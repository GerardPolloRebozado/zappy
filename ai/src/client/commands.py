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
