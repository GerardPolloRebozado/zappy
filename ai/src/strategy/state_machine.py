import math as pymath
from training.training_env.ZappyEnv import ControllerAction
from src.utils.inventory import Inventory
from src.utils.level_up import ELEVATION_TABLE


def get_survival_action(client, parsed_inv, vision_list, item_name="food"):
    """
    Computes a multi-step Dijkstra path to the nearest resource (food or stone)
    visible in the LOOK cone.
    """
    # If we have a cached path, continue executing it
    if hasattr(client, "path_steps") and client.path_steps:
        return client.path_steps.pop(0)

    if not isinstance(vision_list, list) or len(vision_list) == 0:
        return None

    # Check if target item is already on our current tile (tile 0)
    current_tile = vision_list[0]
    entities = (
        current_tile.strip().split(" ")
        if isinstance(current_tile, str)
        else current_tile
    )
    if item_name in entities:
        if item_name == "food":
            return int(ControllerAction.TAKE_FOOD)
        else:
            # Map stone name to take command
            stone_map = {
                "linemate": ControllerAction.TAKE_LINEMATE,
                "deraumere": ControllerAction.TAKE_DERAUMERE,
                "sibur": ControllerAction.TAKE_SIBUR,
                "mendiane": ControllerAction.TAKE_MENDIANE,
                "phiras": ControllerAction.TAKE_PHIRAS,
                "thystame": ControllerAction.TAKE_THYSTAME,
            }
            return int(stone_map.get(item_name, ControllerAction.FORWARD))

    # Search Look cone for the nearest tile containing the item
    target_tile = -1
    for idx, tile in enumerate(vision_list):
        entities = tile.strip().split(" ") if isinstance(tile, str) else tile
        if item_name in entities:
            target_tile = idx
            break

    if target_tile == -1:
        return None

    # Calculate path steps to look cone tile index
    d = int(pymath.sqrt(target_tile))
    x = target_tile - (d**2 + d)
    path = [int(ControllerAction.FORWARD)] * d

    if x < 0:
        path.insert(0, int(ControllerAction.LEFT))
        path.extend([int(ControllerAction.FORWARD)] * abs(x))
    elif x > 0:
        path.insert(0, int(ControllerAction.RIGHT))
        path.extend([int(ControllerAction.FORWARD)] * x)

    # Append take command at the end of the path
    if item_name == "food":
        path.append(int(ControllerAction.TAKE_FOOD))
    else:
        stone_map = {
            "linemate": ControllerAction.TAKE_LINEMATE,
            "deraumere": ControllerAction.TAKE_DERAUMERE,
            "sibur": ControllerAction.TAKE_SIBUR,
            "mendiane": ControllerAction.TAKE_MENDIANE,
            "phiras": ControllerAction.TAKE_PHIRAS,
            "thystame": ControllerAction.TAKE_THYSTAME,
        }
        path.append(int(stone_map.get(item_name, ControllerAction.FORWARD)))

    client.path_steps = path
    if client.path_steps:
        return client.path_steps.pop(0)
    return None


def has_stones_for_level(parsed_inv, vision_list, level):
    """
    Checks if the agent has the required stones either in its inventory or on the floor.
    """
    if level not in ELEVATION_TABLE:
        return True
    req = ELEVATION_TABLE[level]

    tile_0_stones = {
        "linemate": 0,
        "deraumere": 0,
        "sibur": 0,
        "mendiane": 0,
        "phiras": 0,
        "thystame": 0,
    }
    if isinstance(vision_list, list) and len(vision_list) > 0:
        tile_0 = vision_list[0]
        entities = tile_0.strip().split(" ") if isinstance(tile_0, str) else tile_0
        for entity in entities:
            if entity in tile_0_stones:
                tile_0_stones[entity] += 1

    for stone_name in tile_0_stones:
        inv_count = (
            getattr(parsed_inv, stone_name, 0)
            if isinstance(parsed_inv, Inventory)
            else 0
        )
        floor_count = tile_0_stones[stone_name]
        required_count = getattr(req, stone_name, 0)
        if inv_count + floor_count < required_count:
            return False
    return True


def get_hybrid_action(
    client, parsed_inv, vision_list, current_level, best_dir, best_text, model, obs
):
    """
    State machine selector. Generic for all levels 1-7.
    """
    food_count = parsed_inv.food if isinstance(parsed_inv, Inventory) else 10

    req = ELEVATION_TABLE.get(current_level)

    has_stones_for_next_level = False
    if req is not None and isinstance(parsed_inv, Inventory):
        has_stones_for_next_level = True
        for stone in [
            "linemate",
            "deraumere",
            "sibur",
            "mendiane",
            "phiras",
            "thystame",
        ]:
            required = getattr(req, stone, 0)
            if getattr(parsed_inv, stone, 0) < required:
                has_stones_for_next_level = False
                break

    # State 1: Critical Starvation (food < 4)
    if food_count < 4:
        if hasattr(client, "path_steps") and client.path_steps:
            if client.path_steps[-1] != int(ControllerAction.TAKE_FOOD):
                client.path_steps = []
        survival_action = get_survival_action(client, parsed_inv, vision_list, "food")
        if survival_action is not None:
            return survival_action, "SURVIVAL"
        return int(ControllerAction.FORWARD), "SURVIVAL"

    requires_team = req is not None and req.players > 1

    # State 2: Teammate Calling COME (food >= 4)
    if requires_team and best_dir != 0 and food_count >= 4:
        client.path_steps = []
        if best_dir in [1, 2, 8]:
            return int(ControllerAction.FORWARD), "COORDINATION"
        elif best_dir in [3, 4, 5]:
            return int(ControllerAction.LEFT), "COORDINATION"
        else:
            return int(ControllerAction.RIGHT), "COORDINATION"

    # State 3: Teammate Calling INCANT and together (food >= 4)
    if requires_team and best_dir == 0 and "INCANT" in best_text and food_count >= 4:
        client.path_steps = []
        if has_stones_for_next_level:
            for stone in [
                "linemate",
                "deraumere",
                "sibur",
                "mendiane",
                "phiras",
                "thystame",
            ]:
                required = getattr(req, stone, 0)
                if required > 0 and getattr(parsed_inv, stone, 0) >= 1:
                    set_action_name = f"SET_{stone.upper()}"
                    return int(
                        getattr(ControllerAction, set_action_name)
                    ), "COORDINATION"
        return int(ControllerAction.LOOK), "COORDINATION"

    # State 4: Have all stones, requires teammate, no caller -> Broadcast coordinates (food >= 8)
    if (
        requires_team
        and has_stones_for_next_level
        and best_dir == 0
        and food_count >= 8
    ):
        client.path_steps = []
        return int(ControllerAction.BROADCAST), "COORDINATION"

    # State 5: Emergency Search (food < 8)
    if food_count < 8:
        if hasattr(client, "path_steps") and client.path_steps:
            if client.path_steps[-1] != int(ControllerAction.TAKE_FOOD):
                client.path_steps = []
        survival_action = get_survival_action(client, parsed_inv, vision_list, "food")
        if survival_action is not None:
            return survival_action, "SURVIVAL"
        return int(ControllerAction.FORWARD), "SURVIVAL"

    # State 6: Opportunistic Eating (food < 15)
    if food_count < 15:
        survival_action = get_survival_action(client, parsed_inv, vision_list, "food")
        if survival_action is not None:
            return survival_action, "SURVIVAL"

    # State 7: Active Stone Gathering
    if req is not None:
        missing_stones = []
        if isinstance(parsed_inv, Inventory):
            for stone in [
                "linemate",
                "deraumere",
                "sibur",
                "mendiane",
                "phiras",
                "thystame",
            ]:
                required = getattr(req, stone, 0)
                if getattr(parsed_inv, stone, 0) < required:
                    missing_stones.append(stone)

        for stone in missing_stones:
            stone_action = get_survival_action(client, parsed_inv, vision_list, stone)
            if stone_action is not None:
                return stone_action, "GATHER"

    # State 8: Default Fallback to PPO
    client.path_steps = []
    action, _ = model.predict(obs, deterministic=True)
    action = int(action)
    if action == int(ControllerAction.INCANTATION):
        if not has_stones_for_level(parsed_inv, vision_list, current_level):
            action = int(ControllerAction.FORWARD)
    return action, "PPO_FALLBACK"
