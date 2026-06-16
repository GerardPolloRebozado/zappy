from dataclasses import dataclass
from typing import Dict


@dataclass
class ElevationRequirement:
    players: int
    linemate: int = 0
    deraumere: int = 0
    sibur: int = 0
    mendiane: int = 0
    phiras: int = 0
    thystame: int = 0


ELEVATION_TABLE: Dict[int, ElevationRequirement] = {
    1: ElevationRequirement(players=1, linemate=1),
    2: ElevationRequirement(players=2, linemate=1, deraumere=1, sibur=1),
    3: ElevationRequirement(players=2, linemate=2, sibur=1, phiras=2),
    4: ElevationRequirement(players=4, linemate=1, deraumere=1, sibur=2, phiras=1),
    5: ElevationRequirement(players=4, linemate=1, deraumere=2, sibur=1, mendiane=3),
    6: ElevationRequirement(players=6, linemate=1, deraumere=2, sibur=3, phiras=1),
    7: ElevationRequirement(
        players=6, linemate=2, deraumere=2, sibur=2, mendiane=2, phiras=2, thystame=1
    ),
}


def can_evolve(current_level: int, inventory, players_on_tile: int) -> bool:
    if current_level not in ELEVATION_TABLE:
        return False

    req = ELEVATION_TABLE[current_level]

    return (
        players_on_tile >= req.players
        and inventory.linemate >= req.linemate
        and inventory.deraumere >= req.deraumere
        and inventory.sibur >= req.sibur
        and inventory.mendiane >= req.mendiane
        and inventory.phiras >= req.phiras
        and inventory.thystame >= req.thystame
    )
