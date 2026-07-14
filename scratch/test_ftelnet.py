import asyncio
import websockets

async def test():
    try:
        # Some known ways fTelnetProxy might accept host/port
        # 1: In the URL path
        # 2: In the Sec-WebSocket-Protocol (subprotocols)
        uri = "wss://proxy-us.ftelnet.ca:2083"
        
        async with websockets.connect(uri, subprotocols=["telnet", "telehack.com", "23"]) as ws:
            print("Connected to proxy with subprotocols!")
            await ws.send("help\r\n".encode())
            res = await asyncio.wait_for(ws.recv(), timeout=2.0)
            print("Response:", res)
    except Exception as e:
        print("Failed with subprotocols:", e)

    try:
        uri = "wss://proxy-us.ftelnet.ca:2083/?host=telehack.com&port=23"
        async with websockets.connect(uri) as ws:
            print("Connected to proxy with query string!")
            await ws.send("help\r\n".encode())
            res = await asyncio.wait_for(ws.recv(), timeout=2.0)
            print("Response:", res)
    except Exception as e:
        print("Failed with query string:", e)
        
    try:
        # What if it's passed in the first packet?
        uri = "wss://proxy-us.ftelnet.ca:2083"
        async with websockets.connect(uri) as ws:
            print("Connected to bare URL!")
            await ws.send("telehack.com:23\n".encode())
            res = await asyncio.wait_for(ws.recv(), timeout=2.0)
            print("Response:", res)
    except Exception as e:
        print("Failed with bare URL:", e)

asyncio.run(test())
