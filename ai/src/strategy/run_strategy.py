def run_client(client):
    """
    Loop to handle server communication
    :param client: The AI client instance
    :return: None
    """
    try:
        while not client.is_dead:
            take_decision(client)
            
            while client.messages:
                msg = client.messages.pop(0)
                print(f"Broadcast from {msg['direction']}: {msg['text']}")

    except KeyboardInterrupt:
        pass
    finally:
        client.close()
