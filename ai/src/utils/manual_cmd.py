COMMANDS_MAP = {
    1: "Forward",
    2: "Right",
    3: "Left",
    4: "Look",
    5: "Inventory",
    6: "Broadcast",
    7: "Connect_nbr",
    8: "Fork",
    9: "Eject",
    10: "Take",
    11: "Set",
    12: "Incantation",
}


def print_manual_commands():
    """
    Prints the list of commands
    :return: N/A
    """
    print("\n--- Manual Mode Commands ---")
    for num, cmd in COMMANDS_MAP.items():
        if cmd in ["Broadcast", "Take", "Set"]:
            print(f"  [{num}] {cmd} <arg>")
        else:
            print(f"  [{num}] {cmd}")
    print("  Type 'help' or 'h' to show this menu.")
    print("  Type 'exit' or 'quit' to exit.")
    print("----------------------------\n")
