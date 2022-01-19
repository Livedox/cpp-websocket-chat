import json
import websocket #websocket-client
import random
import string


def generate_random_string(length):
    letters = string.ascii_lowercase
    rand_string = ''.join(random.choice(letters) for i in range(length))
    return rand_string


def on_message(ws, message):
  data = json.loads(message)
  if data["head"] == "accept_message":
    if data["message"][:2].lower() == "-b":
      s = generate_random_string(random.randint(5, 100))
      ws.send(json.dumps({
        "head": "send",
        "message": "Alien friend : " + s
      }))
    

if __name__ == "__main__":
  websocket.enableTrace(True)
  ws = websocket.WebSocketApp("ws://127.0.0.1:8083", on_message=on_message)

  ws.run_forever()