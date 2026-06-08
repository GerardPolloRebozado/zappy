from src.utils import COMMANDS_MAP, print_manual_commands


def run_manual(client):
    """
    Handle manual communication with server
    :param client: The AI client instance
    :return: None
    """
    print_manual_commands()

    try:
        while not client.is_dead:
            # check and print any broadcast messages first
            while client.messages:
                msg = client.messages.pop(0)
                print(f"[Broadcast] From {msg['direction']}: {msg['text']}")

            # prompt user for command
            try:
                user_input = input("\nZappy> ").strip()
            except EOFError:
                print("\nExiting manual mode...")
                break

            if not user_input:
                continue

            parts = user_input.split(maxsplit=1)
            cmd_key = parts[0].lower()
            arg = parts[1] if len(parts) > 1 else ""

            # exit
            if cmd_key in ["exit", "quit", "q"]:
                print("Exiting manual mode...")
                break

            # help
            if cmd_key in ["help", "h"]:
                print_manual_commands()
                continue

            matched_cmd = None

            # matching by command number
            try:
                cmd_num = int(cmd_key)
                if cmd_num in COMMANDS_MAP:
                    matched_cmd = COMMANDS_MAP[cmd_num]
            except ValueError:
                pass

            # matching by command name case-insensitively
            if not matched_cmd:
                for num, name in COMMANDS_MAP.items():
                    if name.lower() == cmd_key:
                        matched_cmd = name
                        break

            if not matched_cmd:
                print(
                    f"Error: Unknown command '{parts[0]}'. Type 'help' to see available commands."
                )
                continue

            # execute command
            response = None
            try:
                match matched_cmd:
                    case "Forward":
                        response = client.forward()
                    case "Right":
                        response = client.right()
                    case "Left":
                        response = client.left()
                    case "Look":
                        response = client.look()
                    case "Inventory":
                        response = client.inventory()
                    case "Broadcast":
                        if not arg:
                            arg = input("Enter text to broadcast: ").strip()
                        response = client.broadcast(arg)
                    case "Connect_nbr":
                        response = client.connect_nbr()
                    case "Fork":
                        response = client.fork()
                    case "Eject":
                        response = client.eject()
                    case "Take":
                        if not arg:
                            arg = input("Enter object to take: ").strip()
                        response = client.take(arg)
                    case "Set":
                        if not arg:
                            arg = input("Enter object to set: ").strip()
                        response = client.set(arg)
                    case "Incantation":
                        response = client.incantation()

                # response from server
                if response is not None:
                    print(f"Server Response: {response}")
                else:
                    print("No response (connection may have been closed).")
                    break

            except Exception as e:
                print(f"Error executing command: {e}")

            if client.is_dead:
                print("You died!")
                break

    except KeyboardInterrupt:
        print("\nExiting manual mode...")
    finally:
        client.close()
