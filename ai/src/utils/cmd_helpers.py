def parse_look(data):
    """
    Parses look string like "[player, food food, , linemate, ...]"
    Returns a list of lists, where each sub-list contains the items on a tile.
    """
    content = data.strip("[]")
    tiles_raw = content.split(",")

    tiles = []
    for tile_raw in tiles_raw:
        items = tile_raw.strip().split()
        tiles.append(items)

    return tiles


def parse_broadcast(data):
    """
    Parses broadcast string like "message K, text"
    Returns a dictionary with 'direction' (int) and 'text' (str).
    """
    try:
        parts = data.split(", ", 1)
        direction = int(parts[0].split()[1])
        text = parts[1]
        return {"direction": direction, "text": text}
    except (IndexError, ValueError):
        return None
