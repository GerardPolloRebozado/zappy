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
