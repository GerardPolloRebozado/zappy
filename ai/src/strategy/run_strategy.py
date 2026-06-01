import time

def run_client(client):
    """
    Loop to handle server communication
    :param client: The AI client instance
    :return: None
    """
    try:
        while not client.is_dead:
            while client.messages:
                msg = client.messages.pop(0)
                print(f"Broadcast: {msg}")

            # result = client.look()
            # print(f"Received: {result}")

            time.sleep(1)

    except KeyboardInterrupt:
        pass
    finally:
        client.close()
