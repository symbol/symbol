import asyncio
import json
import os
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.sc import Amount
from websockets import connect

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')

MONITOR_ADDRESS = os.getenv(
	'MONITOR_ADDRESS','TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I'
)
print(f'Monitoring address: {MONITOR_ADDRESS}')

SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000'
)
facade = SymbolFacade('testnet')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))


async def main():
	async with connect(WS_URL) as websocket:
		# Connect to WebSocket
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'Connected to {WS_URL} with uid {uid}')

		# Subscribe to status channel
		channel = f'status/{MONITOR_ADDRESS}'
		await websocket.send(json.dumps(
			{'uid': uid, 'subscribe': channel}
		))
		print('Subscribed to status channel')

		# Build a transfer transaction with a non-existent mosaic
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
			'mosaics': [{
				'mosaic_id': generate_mosaic_alias_id('symbol.unknown'),
				'amount': 1
			}],
		})
		transaction.fee = Amount(fee_mult * transaction.size)

		signature = facade.sign_transaction(signer_key_pair, transaction)
		json_payload = facade.transaction_factory.attach_signature(
			transaction, signature)
		transaction_hash = str(facade.hash_transaction(transaction))

		announce_request = urllib.request.Request(
			f'{NODE_URL}/transactions',
			data=json_payload.encode(),
			headers={'Content-Type': 'application/json'},
			method='PUT'
		)
		with urllib.request.urlopen(announce_request) as resp:
			resp.read()
		print(f'Announced transaction {transaction_hash[:16]}...')

		# Wait for error via WebSocket
		async for raw_message in websocket:
			msg = json.loads(raw_message)
			tx_hash = msg['data']['hash']
			code = msg['data']['code']
			print(
				f'Transaction {tx_hash[:16]}... '
				f'rejected with code: {code}'
			)

			if tx_hash == transaction_hash:
				break

		# Unsubscribe before closing
		await websocket.send(json.dumps(
			{'uid': uid, 'unsubscribe': channel}
		))
		print('Unsubscribed from status channel')


try:
	asyncio.run(main())
except Exception as error:
	print(error)
