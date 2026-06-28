import math
from src.utils import ELEVATION_TABLE, can_evolve
from src.utils.logging_levels import logger


def get_missing_resources(level, inventory):
    if level not in ELEVATION_TABLE:
        return []
    req = ELEVATION_TABLE[level]
    missing = []

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
    if tile_index == 0:
        return True

    logger.info("Calculating route to tile")
    d = int(math.sqrt(tile_index))
    x = tile_index - (d**2 + d)

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
    return tile.count("player")


def _find_broadcast(client):
    level_str = f"level {client.level}"
    if hasattr(client, "messages") and client.messages:
        for msg in reversed(client.messages):
            text = msg.get("text", "")
            if level_str in text and (
                text.startswith("Elevation") or text.startswith("Incantation")
            ):
                return msg
    return None


def _has_zero_broadcast(client):
    level_str = f"level {client.level}"
    if hasattr(client, "messages") and client.messages:
        for msg in reversed(client.messages):
            text = msg.get("text", "")
            d = msg.get("direction", -1)
            if (
                d == 0
                and level_str in text
                and (text.startswith("Elevation") or text.startswith("Incantation"))
            ):
                return True
    return False


def take_decision(client):
    # 0 detect level change (post-incantation) → stay together as a group
    prev_level = getattr(client, "_prev_level", client.level)
    client._prev_level = client.level
    if client.level > prev_level:
        logger.info(
            f"Level increased: {prev_level} -> {client.level}. Broadcasting next level..."
        )
        client.broadcast(f"Elevation {client.name} level {client.level}")
        client._is_waiting = True
        client._wait_count = 0
        client.messages.clear()
        return

    client._is_waiting = getattr(client, "_is_waiting", False)

    # steering: only respond to direction>0 broadcasts when not already waiting
    if not client._is_waiting:
        target_msg = _find_broadcast(client)
        if target_msg is not None:
            direction = target_msg.get("direction", -1)
            if direction > 0:
                logger.info(
                    f"Received elevation broadcast from direction {direction}. Steering towards it..."
                )
                if direction in [1, 2, 8]:
                    client.forward()
                elif direction in [3, 4, 5]:
                    client.left()
                elif direction in [6, 7]:
                    client.right()
                client.take("food")
                client.messages.clear()
                return

    # 1 inventory
    inv = client.inventory()
    logger.debug(f"[SERVER] -> {inv}")
    if inv is None or isinstance(inv, str):
        logger.error(f"Failed to get inventory: {inv}")
        return

    # 2 look
    look = client.look()
    logger.debug(f"[SERVER] -> {look}")
    if look is None or isinstance(look, str):
        logger.error(f"Failed to look: {look}")
        return

    current_tile = look[0]
    players_on_tile = count_players_in_tile(current_tile)

    # 3 check if can evolve right now
    if can_evolve(client.level, inv, players_on_tile):
        req = ELEVATION_TABLE[client.level]

        needed_players = req.players
        if needed_players >= 4 and players_on_tile < needed_players + 1:
            client.broadcast(f"Elevation {client.name} level {client.level}")
            if "food" in current_tile and inv.food < 20:
                client.take("food")
            client.messages.clear()
            return

        logger.debug(
            f"[SERVER] -> {client.broadcast(f'Incantation {client.name} level {client.level}')}"
        )
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
        if isinstance(res, str) and res.strip().lower() == "ko":
            logger.info("Incantation failed! Retrieving dropped stones...")
            stones = [
                ("linemate", req.linemate),
                ("deraumere", req.deraumere),
                ("sibur", req.sibur),
                ("mendiane", req.mendiane),
                ("phiras", req.phiras),
                ("thystame", req.thystame),
            ]
            for name, count in stones:
                for _ in range(count):
                    logger.debug(f"[SERVER] -> {client.take(name)}")
        client._is_waiting = False
        client.messages.clear()
        return

    # 4 respond to broadcasts on the same tile — wait as long as the leader is here
    if _has_zero_broadcast(client) or client._is_waiting:
        client._is_waiting = True
        if not _has_zero_broadcast(client):
            logger.info(
                "Waiting but no broadcast — re-broadcasting to maintain contact."
            )
            client.broadcast(f"Elevation {client.name} level {client.level}")
            client.messages.clear()
            return

        client._wait_count = getattr(client, "_wait_count", 0) + 1

        # Periodically scan nearby tiles for missing resources
        if client._wait_count % 3 == 0:
            missing = get_missing_resources(client.level, inv)
            for i, tile in enumerate(look):
                if i > 0 and i <= 3:
                    for item in tile:
                        if item in missing:
                            logger.info(
                                f"Leaving waiting to grab {item} on tile {i}..."
                            )
                            client._is_waiting = False
                            move_to_tile(client, i)
                            client.take(item)
                            client.messages.clear()
                            return

        if "food" in current_tile:
            logger.info("Waiting on leader's tile (taking food)...")
            client.take("food")
        elif inv.food < 3:
            logger.info("Food too low, gave up waiting.")
            client._is_waiting = False
            client.messages.clear()
        else:
            client.take("food")
        client.messages.clear()
        return

    client._is_waiting = False

    # 5 we have enough resources but not enough players, broadcast
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
            client.messages.clear()
            return

    client.messages.clear()

    # 6 search for needed resources
    missing = get_missing_resources(client.level, inv)

    if "food" in current_tile:
        logger.info("Taking food from current tile.")
        client.take("food")
        return

    for item in current_tile:
        if item in missing:
            logger.info(f"Taking {item} from current tile.")
            logger.debug(f"[SERVER] -> {client.take(item)}")
            return

    for i, tile in enumerate(look):
        for item in tile:
            if item in missing:
                logger.info(f"Found {item} on tile {i}. Moving...")
                move_to_tile(client, i)
                return

    logger.info("Nothing interesting in sight. Exploring...")
    logger.debug(f"[SERVER] -> {client.forward()}")
