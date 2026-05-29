def run_client(client):
    """
    Loop to handle server communication
    :param client: The AI client instance
    :return: None
    """
    try:
        while True:
            line = client.receive_line()
            if line is None:
                print("Server closed the connection.")
                break
            if line == "dead":
                print("You died.")
                break
            print(f"Received: {line}")
            # here we will have the logic of using the AI
    except KeyboardInterrupt:
        pass
    finally:
        client.close()
