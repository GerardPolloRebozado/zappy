import time

def run_client(client):
    """
    Loop to handle server communication
    :param client: The AI client instance
    :return: None
    """
    try:
        while not client.is_dead:
            # -- testing
            res = client.look()
            
            # -- if not testing just waits for server to say something
            #res = client.wait_for_response()

            if res is None:
                print("Server closed the connection.")
                break

            if client.is_dead or res == "dead":
                print("The AI has died.")
                break

            print(f"Result: {res}")
            
            while client.messages:
                msg = client.messages.pop(0)
                print(f"Broadcast from {msg['direction']}: {msg['text']}")

            # -- testing command
            time.sleep(1)
            
    except KeyboardInterrupt:
        pass
    finally:
        client.close()
