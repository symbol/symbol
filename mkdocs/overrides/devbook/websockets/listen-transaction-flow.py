import asyncio
import json
import os

from websockets import connect

NODE_URL = os.getenv(
	'NODE_URL',
	'https://reference.symboltest.net:3001'
)
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')

MONITOR_ADDRESS = os.getenv(
	'MONITOR_ADDRESS',
	'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'
)
print(f'Monitoring address: {MONITOR_ADDRESS}')


async def main():
	async with connect(WS_URL) as websocket:
		# Connect and receive uid
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'Connected to {WS_URL} with uid {uid}')

		# Subscribe to transaction channels
		channels = [
			f'confirmedAdded/{MONITOR_ADDRESS}',
			f'unconfirmedAdded/{MONITOR_ADDRESS}',
			f'unconfirmedRemoved/{MONITOR_ADDRESS}',
		]
		for channel in channels:
			await websocket.send(json.dumps(
				{'uid': uid, 'subscribe': channel}
			))
			name = channel.split('/')[0]
			print(f'Subscribed to {name} channel')

		# Handle incoming messages
		try:
			async for raw_message in websocket:
				message = json.loads(raw_message)
				topic = message['topic']
				transaction_hash = message['data']['meta']['hash']
				name = topic.split('/')[0]
				print(f'{name}: hash={transaction_hash[:16]}...')

		# Unsubscribe on exit
		finally:
			for channel in channels:
				await websocket.send(json.dumps({
					'uid': uid,
					'unsubscribe': channel
				}))
			print('Unsubscribed from all channels')


try:
	asyncio.run(main())
except KeyboardInterrupt:
	pass
except Exception as error:
	print(error)
