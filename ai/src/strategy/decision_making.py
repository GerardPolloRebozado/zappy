import math
import random

from src.utils import ELEVATION_TABLE
from src.utils.logging_levels import logger

STONES = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

# Food units left in the inventory. Each unit feeds the player for 126 time
# units, so these thresholds trade a small survival buffer against the time
# spent foraging instead of gathering/grouping.
FOOD_CRITICAL = 18  # below this, abandon everything and find food
FOOD_COMFORT = 50  # opportunistically top up to this while convenient

# Highest level we actively try to reach. Reaching level 6 only requires
# coordinating 4 players (the level 4->5 and 5->6 incantations); level 7 needs
# 6 players gathered at once which is far less reliable.
GOAL_LEVEL = 6

# How long a sub-group that is too small to incant waits before attempting to
# merge with another group it can hear (randomised to break the symmetry of two
# equal groups calling each other forever).
MERGE_TIMEOUT = 12
# How long a lone caller waits for company before wandering to look for others.
LONE_WANDER = 10


def _stone(obj, name):
    return getattr(obj, name, 0)


def _missing_current(level, inv):
    """Stones still missing for the current level's incantation."""
    req = ELEVATION_TABLE.get(level)
    if req is None:
        return set()
    return {s for s in STONES if _stone(inv, s) < _stone(req, s)}


def _has_current_stones(level, inv):
    req = ELEVATION_TABLE.get(level)
    if req is None:
        return False
    return all(_stone(inv, s) >= _stone(req, s) for s in STONES)


def _stockpile_target(level):
    """Cumulative stones needed to climb from `level` up to GOAL_LEVEL.

    Carrying this whole set lets a single player supply every incantation on the
    way to the goal, so a grouped team never has to disperse to find stones.
    """
    target = {s: 0 for s in STONES}
    for lvl in range(level, GOAL_LEVEL):
        req = ELEVATION_TABLE.get(lvl)
        if req is None:
            continue
        for s in STONES:
            target[s] += _stone(req, s)
    return target


def _stockpile_missing(level, inv):
    target = _stockpile_target(level)
    return {s for s in STONES if _stone(inv, s) < target[s]}


def _scan_calls(client, level):
    """Inspect heard broadcasts for rendezvous signals at our level.

    Rendezvous calls carry the caller's group size ("come <level> <count>") so
    that smaller groups can deterministically flow into the largest audible
    group instead of two equal groups calling each other forever.

    Returns (best_dir, best_count, go_same_tile):
      best_dir:     direction (1-8) of the largest "come" group heard from
                    another tile, or None.
      best_count:   advertised player count of that group (0 if none).
      go_same_tile: True if an incantation is starting on our tile.
    """
    best_dir = None
    best_count = 0
    go_same_tile = False
    if not getattr(client, "messages", None):
        return best_dir, best_count, go_same_tile

    for msg in client.messages:
        text = msg.get("text", "")
        direction = msg.get("direction", -1)
        parts = text.split()
        if len(parts) < 2:
            continue
        kind, lvl = parts[0], parts[1]
        if not lvl.isdigit() or int(lvl) != level:
            continue
        if kind == "go" and direction == 0:
            go_same_tile = True
        elif kind == "come" and direction > 0:
            count = int(parts[2]) if len(parts) > 2 and parts[2].isdigit() else 1
            if count > best_count:
                best_count = count
                best_dir = direction
    return best_dir, best_count, go_same_tile


def move_to_tile(client, tile_index):
    """Walk to a tile from the level-relative look index using its cone math."""
    if tile_index == 0:
        return True

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


def _move_to_visible(client, look, items):
    """Move to the nearest visible tile holding any wanted item."""
    for i, tile in enumerate(look):
        if i == 0:
            continue
        for item in tile:
            if item in items:
                logger.info(f"Heading to {item} on tile {i}.")
                move_to_tile(client, i)
                return True
    return False


def _steer(client, direction):
    """Take one step toward a sound coming from relative direction `direction`."""
    if direction in (1, 2, 8):
        client.forward()
    elif direction in (3, 4, 5):
        client.left()
    elif direction in (6, 7):
        client.right()
    else:
        client.forward()


def _explore(client):
    """Wander to discover resources and other players."""
    client._explore_step = getattr(client, "_explore_step", 0) + 1
    if client._explore_step % 4 == 0:
        client.right()
    else:
        client.forward()


def count_players_in_tile(tile):
    return tile.count("player")


def _perform_incantation(client, level, tile):
    req = ELEVATION_TABLE[level]
    logger.info(f"Starting incantation for level {level} -> {level + 1}.")
    client.broadcast(f"go {level}")

    for s in STONES:
        for _ in range(_stone(req, s)):
            logger.debug(f"[SERVER] -> {client.set(s)}")

    res = client.incantation()
    logger.debug(f"[SERVER] -> incantation: {res}")
    if isinstance(res, str) and res.strip().lower() == "ko":
        logger.info("Incantation failed, retrieving dropped stones.")
        for s in STONES:
            for _ in range(_stone(req, s)):
                logger.debug(f"[SERVER] -> {client.take(s)}")


def take_decision(client):
    if client.is_dead:
        return

    inv = client.inventory()
    if inv is None or isinstance(inv, str):
        logger.error(f"Failed to get inventory: {inv}")
        return

    look = client.look()
    if look is None or isinstance(look, str):
        logger.error(f"Failed to look: {look}")
        return

    tile = look[0]
    players_here = count_players_in_tile(tile)
    level = client.level
    req = ELEVATION_TABLE.get(level)
    best_dir, best_count, go_same_tile = _scan_calls(client, level)

    # 0. Survival: nothing matters if we starve.
    if inv.food < FOOD_CRITICAL:
        client.messages.clear()
        if "food" in tile:
            logger.info("Critical food, eating.")
            client.take("food")
            return
        if _move_to_visible(client, look, ["food"]):
            return
        _explore(client)
        return

    # 1. Incantate now if the tile already satisfies our level's requirement.
    if req and players_here >= req.players and _has_current_stones(level, inv):
        client.messages.clear()
        _perform_incantation(client, level, tile)
        return

    # 2. A neighbour on our tile is starting an incantation: hold position so we
    #    are still counted when it resolves (the level-up event will reach us).
    if go_same_tile:
        client.messages.clear()
        if "food" in tile:
            client.take("food")
        else:
            client.look()
        return

    # 3. We hold our level's stones: rendezvous with same-level players.
    #    Calls advertise our group size so the team coalesces into a single
    #    growing pile instead of several small ones that never reach the player
    #    count a high-level incantation needs.
    if req and _has_current_stones(level, inv):
        client.broadcast(f"come {level} {players_here}")

        join_bigger = best_dir is not None and best_count > players_here
        # Two equal-sized groups would otherwise anchor forever waiting for each
        # other; after a grace period one of them randomly moves to merge.
        client._wait = getattr(client, "_wait", 0) + 1
        merge_equal = (
            best_dir is not None
            and best_count == players_here
            and players_here < req.players
            and client._wait > MERGE_TIMEOUT
            and random.random() < 0.3
        )

        if join_bigger or merge_equal:
            client._wait = 0
            client.messages.clear()
            _steer(client, best_dir)
            return

        # Anchor and let others converge. Keep eating; a lone caller with no
        # answer for a while wanders to bump into stragglers.
        client.messages.clear()
        if "food" in tile and inv.food < FOOD_COMFORT:
            client.take("food")
        elif players_here <= 1 and client._wait > LONE_WANDER:
            client._wait = 0
            _explore(client)
        else:
            client.look()
        return

    client._wait = 0

    # 4. We still need stones: gather, grabbing useful future stones in passing.
    missing_now = _missing_current(level, inv)
    wants = missing_now | _stockpile_missing(level, inv)

    if "food" in tile and inv.food < FOOD_COMFORT:
        logger.info("Topping up food.")
        client.messages.clear()
        client.take("food")
        return

    for s in STONES:
        if s in tile and s in wants:
            logger.info(f"Picking up {s}.")
            client.messages.clear()
            client.take(s)
            return

    targets = set(wants)
    if inv.food < FOOD_COMFORT:
        targets.add("food")
    if _move_to_visible(client, look, targets):
        client.messages.clear()
        return

    client.messages.clear()
    _explore(client)
