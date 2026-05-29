import time

def run_client(client):
    """
    Loop to handle server communication
    :param client: The AI client instance
    :return: None
    """
    try:
        while True:
            # -- test forward
            # client.forward()

            line = client.receive_line()
            if line is None:
                print("Server closed the connection.")
                break
            print(f"Received: {line}")
            if line == "dead":
                print("You died.")
                break
            # -- this is here for now to avoid spamming the server with the forward command
            # time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        client.close()
