import math
from src.utils import ELEVATION_TABLE, can_evolve
from src.utils.logging_levels import logger


def get_missing_resources(level, inventory):
    """
    Returns a list of resource names that are still needed for the next level.
    """
    if level not in ELEVATION_TABLE:
        return []
    req = ELEVATION_TABLE[level]
    missing = []

    # make sure player eats as to not die
    if inventory.food < 20:
        missing.append("food")

    if inventory.linemate < req.linemate:
        missing.append("linemate")
    if inventory.deraumere < req.deraumere:
        missing.append("deraumere")
    if inventory.sibur < req.sibur:
        missing.append("sibur")
    if inventory.mendiane < req.mendiane:
        missing.append("mendiane")
    if inventory.phiras < req.phiras:
        missing.append("phiras")
    if inventory.thystame < req.thystame:
        missing.append("thystame")

    return missing


def move_to_tile(client, tile_index):
    """
    Moves the client to the tile indicated by the index in the 'look' command.
    """
    if tile_index == 0:
        return True

    logger.info("Calculating route to tile")
    d = int(math.sqrt(tile_index))
    x = tile_index - (d**2 + d)

    # move forward d times, then turn and move x times
    for _ in range(d):
        logger.debug(f"[SERVER] -> {client.forward()}")

    if x < 0:
        logger.debug(f"[SERVER] -> {client.left()}")
        for _ in range(abs(x)):
            logger.debug(f"[SERVER] -> {client.forward()}")
    elif x > 0:
        logger.debug(f"[SERVER] -> {client.right()}")
        for _ in range(x):
            logger.debug(f"[SERVER] -> {client.forward()}")
    return True


def count_players_in_tile(tile):
    """
    Counts the number of players in the tile.
    :param tile: the tile to count
    :return: number of players
    """
    return tile.count("player")


def take_decision(client):
    """
    Main decision making logic.
    """
    # 1 check inventory first
    inv = client.inventory()
    logger.debug(f"[SERVER] -> {inv}")
    if inv is None or isinstance(inv, str):
        logger.error(f"Failed to get inventory: {inv}")
        return

    # 2 check if can evolve right now
    look = client.look()
    logger.debug(f"[SERVER] -> {look}")
    if look is None or isinstance(look, str):
        logger.error(f"Failed to look: {look}")
        return

    current_tile = look[0]
    players_on_tile = count_players_in_tile(current_tile)

    if can_evolve(client.level, inv, players_on_tile):
        req = ELEVATION_TABLE[client.level]
        logger.info(f"Evolving to level {client.level + 1}! Dropping resources...")
        for _ in range(req.linemate):
            logger.debug(f"[SERVER] -> {client.set('linemate')}")
        for _ in range(req.deraumere):
            logger.debug(f"[SERVER] -> {client.set('deraumere')}")
        for _ in range(req.sibur):
            logger.debug(f"[SERVER] -> {client.set('sibur')}")
        for _ in range(req.mendiane):
            logger.debug(f"[SERVER] -> {client.set('mendiane')}")
        for _ in range(req.phiras):
            logger.debug(f"[SERVER] -> {client.set('phiras')}")
        for _ in range(req.thystame):
            logger.debug(f"[SERVER] -> {client.set('thystame')}")
        res = client.incantation()
        logger.debug(f"[SERVER] -> {res}")
        return

    # 3 check broadcast messages from teammates first before waiting or resource hunting
    target_msg = None
    if hasattr(client, "messages") and client.messages:
        for msg in reversed(client.messages):
            if msg.get("text") == f"Elevation {client.name} level {client.level}":
                target_msg = msg
                break

    if target_msg is not None:
        direction = target_msg.get("direction", -1)
        if direction == 0:
            logger.info("On the same tile as elevation leader. Waiting...")
            if "food" in current_tile and inv.food < 20:
                logger.info("Taking food while waiting...")
                logger.debug(f"[SERVER] -> {client.take('food')}")
            else:
                logger.debug(f"[SERVER] -> {client.inventory()}")
            client.messages.clear()
            return
        elif direction > 0:
            logger.info(
                f"Received elevation broadcast from direction {direction}. Steering towards it..."
            )
            if direction in [1, 2, 8]:
                logger.debug(f"[SERVER] -> {client.forward()}")
            elif direction in [3, 4, 5]:
                logger.debug(f"[SERVER] -> {client.left()}")
            elif direction in [6, 7]:
                logger.debug(f"[SERVER] -> {client.right()}")
            client.messages.clear()
            return

    # 4 we have enough resources but not enough players, broadcast
    if client.level in ELEVATION_TABLE:
        req = ELEVATION_TABLE[client.level]
        has_resources = (
            inv.linemate >= req.linemate
            and inv.deraumere >= req.deraumere
            and inv.sibur >= req.sibur
            and inv.mendiane >= req.mendiane
            and inv.phiras >= req.phiras
            and inv.thystame >= req.thystame
        )
        if has_resources and players_on_tile < req.players:
            logger.info(
                f"Waiting for players for level {client.level}. Current: {players_on_tile}/{req.players}"
            )
            logger.debug(
                f"[SERVER] -> {client.broadcast(f'Elevation {client.name} level {client.level}')}"
            )
            if "food" in current_tile:
                logger.info("Taking food while waiting...")
                logger.debug(f"[SERVER] -> {client.take('food')}")
            return

    # 4 search for needed resources
    missing = get_missing_resources(client.level, inv)

    # P1 take what's on current tile if needed
    for item in current_tile:
        if item in missing:
            logger.info(f"Taking {item} from current tile.")
            logger.debug(f"[SERVER] -> {client.take(item)}")
            return

    # P2 move to closest tile with needed resource
    for i, tile in enumerate(look):
        for item in tile:
            if item in missing:
                logger.info(f"Found {item} on tile {i}. Moving...")
                move_to_tile(client, i)
                return

    # P3 nothing needed in sight, explore
    logger.info("Nothing interesting in sight. Exploring...")
    logger.debug(f"[SERVER] -> {client.forward()}")
