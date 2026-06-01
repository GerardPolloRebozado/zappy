class Inventory:
    def __init__(self, food=0, linemate=0, deraumere=0, sibur=0, mendiane=0, phiras=0, thystame=0):
        """
        Initializes an Inventory object
        :param food: the food that gives energy to the player
        :param linemate: stone
        :param deraumere: stone
        :param sibur: stone
        :param mendiane: stone
        :param phiras: stone
        :param thystame: stone
        """
        self.food = food
        self.linemate = linemate
        self.deraumere = deraumere
        self.sibur = sibur
        self.mendiane = mendiane
        self.phiras = phiras
        self.thystame = thystame

    @classmethod
    def from_string(cls, data):
        """
        Parses inventory string like "[food 10, linemate 2, ...]"
        """
        content = data.strip("[]")
        items = [item.strip() for item in content.split(",")]

        inv_dict = {}
        for item in items:
            parts = item.split()
            if len(parts) == 2:
                name, count = parts
                inv_dict[name] = int(count)

        return cls(
            food=inv_dict.get("food", 0),
            linemate=inv_dict.get("linemate", 0),
            deraumere=inv_dict.get("deraumere", 0),
            sibur=inv_dict.get("sibur", 0),
            mendiane=inv_dict.get("mendiane", 0),
            phiras=inv_dict.get("phiras", 0),
            thystame=inv_dict.get("thystame", 0)
        )

    def __repr__(self):
        """
        representation. how the object is printed
        :return: A string representation of the inventory
        """
        return (f"Inventory(food={self.food}, linemate={self.linemate}, "
                f"deraumere={self.deraumere}, sibur={self.sibur}, "
                f"mendiane={self.mendiane}, phiras={self.phiras}, "
                f"thystame={self.thystame})")
