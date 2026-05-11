import asyncio
import json
import os

from websockets import connect

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')


async def main():
	async with connect(WS_URL) as websocket:  # [>step-1]
		# Connect to websocket endpoint
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'Connected to {WS_URL} with uid {uid}')
	# [<step-1]
		# Subscribe to block channel [>step-2]
		await websocket.send(json.dumps(
			{'uid': uid, 'subscribe': 'block'}))
		print('Subscribed to block channel')

		# Subscribe to finalizedBlock channel
		await websocket.send(json.dumps(
			{'uid': uid, 'subscribe': 'finalizedBlock'}))
		print('Subscribed to finalizedBlock channel')
		# [<step-2]
		# Handle incoming messages [>step-3]
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
		# [<step-3]
		# Unsubscribe on exit [>step-4]
		finally:
			await websocket.send(json.dumps(
				{'uid': uid, 'unsubscribe': 'block'}))
			await websocket.send(json.dumps(
				{'uid': uid, 'unsubscribe': 'finalizedBlock'}))
			print('Unsubscribed from all channels')

		# [<step-4]
try:
	asyncio.run(main())
except KeyboardInterrupt:
	pass
except Exception as error:
	print(error)
