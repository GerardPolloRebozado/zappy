from enum import Enum, IntEnum


class ControllerAction(IntEnum):
    FORWARD = 0
    LEFT = 1
    RIGHT = 2
    LOOK = 3
    INVENTORY = 4
    BROADCAST = 5
    CONNECT_NBR = 6
    FORK = 7
    EJECT = 8
    TAKE_FOOD = 9
    TAKE_LINEMATE = 10
    TAKE_DERAUMERE = 11
    TAKE_SIBUR = 12
    TAKE_MENDIANE = 13
    TAKE_PHIRAS = 14
    TAKE_THYSTAME = 15
    SET_FOOD = 16
    SET_LINEMATE = 17
    SET_DERAUMERE = 18
    SET_SIBUR = 19
    SET_MENDIANE = 20
    SET_PHIRAS = 21
    SET_THYSTAME = 22
    INCANTATION = 23


class ZappyAction(Enum):
    """
    Enum mapping actions to their corresponding Zappy commands logic.
    """

    FORWARD = "Forward"
    LEFT = "Left"
    RIGHT = "Right"
    LOOK = "Look"
    INVENTORY = "Inventory"
    BROADCAST = "Broadcast"
    CONNECT_NBR = "Connect_nbr"
    FORK = "Fork"
    EJECT = "Eject"
    TAKE = "Take"
    SET = "Set"
    INCANTATION = "Incantation"
