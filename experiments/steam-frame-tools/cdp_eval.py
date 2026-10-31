#!/usr/bin/env python3
import argparse
import json
import urllib.request

import websocket


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("expression")
    parser.add_argument("--title", default="SharedJSContext")
    parser.add_argument("--port", type=int, default=18080)
    args = parser.parse_args()

    with urllib.request.urlopen(f"http://127.0.0.1:{args.port}/json/list") as response:
        pages = json.load(response)
    page = next(item for item in pages if item.get("title") == args.title)
    ws_url = page["webSocketDebuggerUrl"].replace("127.0.0.1:8080", f"127.0.0.1:{args.port}")
    ws = websocket.create_connection(ws_url, origin=f"http://127.0.0.1:{args.port}")
    try:
        ws.send(json.dumps({
            "id": 1,
            "method": "Runtime.evaluate",
            "params": {
                "expression": args.expression,
                "awaitPromise": True,
                "returnByValue": True,
            },
        }))
        while True:
            message = json.loads(ws.recv())
            if message.get("id") == 1:
                print(json.dumps(message, ensure_ascii=False, indent=2))
                break
    finally:
        ws.close()


if __name__ == "__main__":
    main()
