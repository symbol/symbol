import asyncio
import json
import os

from websockets import connect

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')


async def main():
	async with connect(WS_URL) as websocket:
		# Connect to websocket endpoint
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'Connected to {WS_URL} with uid {uid}')

		# Subscribe to block channel
		await websocket.send(json.dumps(
			{'uid': uid, 'subscribe': 'block'}))
		print('Subscribed to block channel')

		# Subscribe to finalizedBlock channel
		await websocket.send(json.dumps(
			{'uid': uid, 'subscribe': 'finalizedBlock'}))
		print('Subscribed to finalizedBlock channel')

		# Handle incoming messages
		try:
			async for raw_message in websocket:
				message = json.loads(raw_message)
				topic = message['topic']

				if topic == 'block':
					block = message['data']['block']
					block_meta = message['data']['meta']
					print(
						f'New block: height={int(block["height"]):,}'
						f' hash={block_meta["hash"][:16]}...'
					)

				if topic == 'finalizedBlock':
					finalized = message['data']
					print(
						f'Finalized: height={int(finalized["height"]):,}'
						f' hash={finalized["hash"][:16]}...'
					)

		# Unsubscribe on exit
		finally:
			await websocket.send(json.dumps(
				{'uid': uid, 'unsubscribe': 'block'}))
			await websocket.send(json.dumps(
				{'uid': uid, 'unsubscribe': 'finalizedBlock'}))
			print('Unsubscribed from all channels')


try:
	asyncio.run(main())
except KeyboardInterrupt:
	pass
except Exception as error:
	print(error)
