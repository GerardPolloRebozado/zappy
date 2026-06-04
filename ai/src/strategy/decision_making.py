import math

def get_missing_resources(level, inventory):
    """
    Returns a list of resource names that are still needed for the next level.
    """
    if level not in ELEVATION_TABLE:
        return []
    req = ELEVATION_TABLE[level]
    missing = []

    # make sure player eats as to not die
    if inventory.food < 10:
        missing.append("food")
        
    if inventory.linemate < req.linemate: missing.append("linemate")
    if inventory.deraumere < req.deraumere: missing.append("deraumere")
    if inventory.sibur < req.sibur: missing.append("sibur")
    if inventory.mendiane < req.mendiane: missing.append("mendiane")
    if inventory.phiras < req.phiras: missing.append("phiras")
    if inventory.thystame < req.thystame: missing.append("thystame")
    
    return missing

def move_to_tile(client, tile_index):
    """
    Moves the client to the tile indicated by the index in the 'look' command.
    """
    if tile_index == 0:
        return True

    d = int(math.sqrt(tile_index))
    x = tile_index - (d**2 + d)
    
    # move forward d times, then turn and move x times
    for _ in range(d):
        client.forward()
    
    if x < 0:
        client.left()
        for _ in range(abs(x)):
            client.forward()
    elif x > 0:
        client.right()
        for _ in range(x):
            client.forward()
    return True

def count_players_in_tile(tile):
    return tile.count("player")

def take_decision(client):
    """
    Main decision making logic.
    """
    # 1 check inventory first
    inv = client.inventory()
    if inv is None or isinstance(inv, str):
        return

    # 2 check if can evolve right now
    look = client.look()
    if look is None or isinstance(look, str):
        return
        
    current_tile = look[0]
    players_on_tile = count_players_in_tile(current_tile)
    
    if can_evolve(client.level, inv, players_on_tile):
        client.incantation()
        return

    # 3 we have enough resources but not enough players, broadcast
    if client.level in ELEVATION_TABLE:
        req = ELEVATION_TABLE[client.level]
        has_resources = (
            inv.linemate >= req.linemate and
            inv.deraumere >= req.deraumere and
            inv.sibur >= req.sibur and
            inv.mendiane >= req.mendiane and
            inv.phiras >= req.phiras and
            inv.thystame >= req.thystame
        )
        if has_resources and players_on_tile < req.players:
            # call for players
            client.broadcast(f"Elevation level {client.level}")
            return

    # 4 search for needed resources
    missing = get_missing_resources(client.level, inv)
    
    # P1 take what's on current tile if needed
    for item in current_tile:
        if item in missing:
            client.take(item)
            return

    # P2 move to closest tile with needed resource
    for i, tile in enumerate(look):
        for item in tile:
            if item in missing:
                move_to_tile(client, i)
                return

    # P3 nothing needed in sight, explore
    client.forward()
