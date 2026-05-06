import asyncio
import json
import os
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.Network import NetworkTimestamp
from websockets import connect

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')
# [>step-1]
MONITOR_ADDRESS = os.getenv(
	'MONITOR_ADDRESS',
	'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'
)
print(f'Monitoring address: {MONITOR_ADDRESS}')

SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000'
)
facade = SymbolFacade('testnet')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))
# [<step-1]

async def main():
	async with connect(WS_URL) as websocket: # [>step-2]
		# Connect to WebSocket
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'Connected to {WS_URL} with uid {uid}')
	# [<step-2]
		# Subscribe to transaction channels [>step-3]
		channels = [
			f'unconfirmedAdded/{MONITOR_ADDRESS}',
			f'unconfirmedRemoved/{MONITOR_ADDRESS}',
			f'confirmedAdded/{MONITOR_ADDRESS}',
		]
		for channel in channels:
			await websocket.send(json.dumps(
				{'uid': uid, 'subscribe': channel}
			))
			name = channel.split('/')[0]
			print(f'Subscribed to {name} channel')
		# [<step-3]
		# Build and announce a transfer transaction [>step-4]
		with urllib.request.urlopen(f'{NODE_URL}/node/time') as resp:
			time_json = json.loads(resp.read().decode())
			timestamp = NetworkTimestamp(int(
				time_json['communicationTimestamps']['receiveTimestamp']))

		with urllib.request.urlopen(
			f'{NODE_URL}/network/fees/transaction'
		) as resp:
			fee_json = json.loads(resp.read().decode())
			fee_mult = max(
				fee_json['medianFeeMultiplier'],
				fee_json['minFeeMultiplier'])

		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
			'signer_public_key': signer_key_pair.public_key,
			'deadline': timestamp.add_hours(2).timestamp,
			'recipient_address': MONITOR_ADDRESS,
		})
		transaction.fee = Amount(fee_mult * transaction.size)

		signature = facade.sign_transaction(signer_key_pair, transaction)
		json_payload = facade.transaction_factory.attach_signature(
			transaction, signature)
		transaction_hash = str(facade.hash_transaction(transaction)) # [<step-4]
		# [>step-5]
		announce_request = urllib.request.Request(
			f'{NODE_URL}/transactions',
			data=json_payload.encode(),
			headers={'Content-Type': 'application/json'},
			method='PUT'
		)
		with urllib.request.urlopen(announce_request) as resp:
			resp.read()
		print(f'Announced transaction {transaction_hash[:16]}...')

		# Wait for confirmation via WebSocket
		async for raw_message in websocket:
			message = json.loads(raw_message)
			topic = message['topic']
			message_hash = message['data']['meta']['hash']
			name = topic.split('/')[0]
			print(f'{name}: hash={message_hash[:16]}...')

			if (name == 'confirmedAdded'
					and message_hash == transaction_hash):
				print(f'Transaction {transaction_hash[:16]}... confirmed')
				break
		# [<step-5]
		# Unsubscribe before closing [>step-6]
		for channel in channels:
			await websocket.send(json.dumps({
				'uid': uid,
				'unsubscribe': channel
			}))
		print('Unsubscribed from all channels')
		# [<step-6]

try:
	asyncio.run(main())
except Exception as error:
	print(error)
