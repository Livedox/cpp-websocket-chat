from transformers import GPT2LMHeadModel, GPT2Tokenizer
import json
import websocket #websocket-client


def load_tokenizer_and_model(model_name_or_path):
  return GPT2Tokenizer.from_pretrained(model_name_or_path), GPT2LMHeadModel.from_pretrained(model_name_or_path)


def generate(
    model, tok, text,
    do_sample=True, max_length=100, repetition_penalty=5.0,
    top_k=5, top_p=0.95, temperature=1,
    num_beams=None,
    no_repeat_ngram_size=3
    ):
  input_ids = tok.encode(text, return_tensors="pt")
  out = model.generate(
      input_ids,
      max_length=max_length,
      repetition_penalty=repetition_penalty,
      do_sample=do_sample,
      top_k=top_k, top_p=top_p, temperature=temperature,
      num_beams=num_beams, no_repeat_ngram_size=no_repeat_ngram_size
      )
  return list(map(tok.decode, out))

tok, model = load_tokenizer_and_model("sberbank-ai/rugpt2large")


def on_message(ws, message):
  data = json.loads(message)
  if data["head"] == "accept_message":
    if data["message"][:2].lower() == "-f":
      generated = generate(model, tok, data["message"][2:].strip(), num_beams=10)
      ws.send(json.dumps({
        "head": "send",
        "message": "Friend: " + generated[0]
      }))
    

if __name__ == "__main__":
  websocket.enableTrace(True)
  ws = websocket.WebSocketApp("ws://127.0.0.1:8083", on_message=on_message)

  ws.run_forever()