import asyncio
import json
import os
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp
from websockets import connect

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
WS_URL = NODE_URL.replace('http', 'ws', 1) + '/ws'
print(f'Using node {NODE_URL}')
# [>step-1]
ACCOUNT_A_PRIVATE_KEY = os.getenv(
	'ACCOUNT_A_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
ACCOUNT_B_PRIVATE_KEY = os.getenv(
	'ACCOUNT_B_PRIVATE_KEY',
	'1111111111111111111111111111111111111111111111111111111111111111')

facade = SymbolFacade('testnet')
account_a_key_pair = SymbolFacade.KeyPair(
	PrivateKey(ACCOUNT_A_PRIVATE_KEY))
account_b_key_pair = SymbolFacade.KeyPair(
	PrivateKey(ACCOUNT_B_PRIVATE_KEY))
account_a_address = facade.network.public_key_to_address(
	account_a_key_pair.public_key)
account_b_address = facade.network.public_key_to_address(
	account_b_key_pair.public_key)
print(f'Account A: {account_a_address}')
print(f'Account B: {account_b_address}')  # [<step-1]


async def main():
	# Fetch current network time
	with urllib.request.urlopen(
		f'{NODE_URL}/node/time'
	) as resp:
		time_json = json.loads(resp.read().decode())
		timestamp = NetworkTimestamp(int(
			time_json['communicationTimestamps']['receiveTimestamp']))

	# Fetch recommended fee multiplier
	with urllib.request.urlopen(
		f'{NODE_URL}/network/fees/transaction'
	) as resp:
		fee_json = json.loads(resp.read().decode())
		fee_multiplier = max(
			fee_json['medianFeeMultiplier'],
			fee_json['minFeeMultiplier'])

	# [Account A] Build embedded transactions for the swap [>step-2]
	embedded_tx_1 = (
		facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction_v1',
			'signer_public_key': account_a_key_pair.public_key,
			'recipient_address': account_b_address,
			'mosaics': [{
				'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
				'amount': 10_000_000
			}]
		}))

	custom_mosaic_id = 0x6D1314BE751B62C2
	embedded_tx_2 = (
		facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction_v1',
			'signer_public_key': account_b_key_pair.public_key,
			'recipient_address': account_a_address,
			'mosaics': [{
				'mosaic_id': custom_mosaic_id,
				'amount': 1
			}]
		}))

	# Build the bonded aggregate transaction
	embedded_txs = [embedded_tx_1, embedded_tx_2]
	bonded_tx = facade.transaction_factory.create({
		'type': 'aggregate_bonded_transaction_v3',
		'signer_public_key': account_a_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_txs),
		'transactions': embedded_txs
	})
	bonded_tx.fee = Amount(fee_multiplier * (bonded_tx.size + 104))

	# Sign the bonded aggregate
	bonded_signature = facade.sign_transaction(
		account_a_key_pair, bonded_tx)
	bonded_payload = facade.transaction_factory.attach_signature(
		bonded_tx, bonded_signature)
	bonded_hash = facade.hash_transaction(bonded_tx)
	print(
		f'[Account A] Bonded aggregate hash: {str(bonded_hash)[:16]}...')

	# Create the hash lock transaction
	hash_lock = facade.transaction_factory.create({
		'type': 'hash_lock_transaction_v1',
		'signer_public_key':
			account_a_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'mosaic': {
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 10_000_000
		},
		'duration': 100,
		'hash': bonded_hash
	})
	hash_lock.fee = Amount(fee_multiplier * hash_lock.size)
	hash_lock_signature = facade.sign_transaction(
		account_a_key_pair, hash_lock)
	hash_lock_payload = facade.transaction_factory.attach_signature(
		hash_lock, hash_lock_signature)
	hash_lock_hash = facade.hash_transaction(hash_lock)

	# Confirm hash lock via WebSocket
	async with connect(WS_URL) as websocket:
		response = json.loads(
			await websocket.recv())
		uid = response['uid']

		lock_channels = [
			f'confirmedAdded/{account_a_address}',
			f'status/{account_a_address}',
		]
		for channel in lock_channels:
			await websocket.send(json.dumps(
				{'uid': uid, 'subscribe': channel}
			))

		# Announce hash lock
		request = urllib.request.Request(
			f'{NODE_URL}/transactions',
			data=hash_lock_payload.encode(),
			headers={'Content-Type': 'application/json'},
			method='PUT'
		)
		with urllib.request.urlopen(request) as resp:
			resp.read()
		print('[Account A] Announced hash lock '
			f'{str(hash_lock_hash)[:16]}...')

		# Wait for hash lock confirmation
		async for raw_message in websocket:
			message = json.loads(raw_message)
			name = message['topic'].split('/')[0]

			if name == 'confirmedAdded':
				message_hash = message['data']['meta']['hash']
				if message_hash == str(hash_lock_hash):
					print('Hash lock confirmed')
					break

			if name == 'status':
				status_hash = message['data']['hash']
				if status_hash == str(hash_lock_hash):
					raise RuntimeError(
						'Hash lock failed: ' + message['data']['code'])

		for channel in lock_channels:
			await websocket.send(json.dumps({
				'uid': uid,
				'unsubscribe': channel
			}))
# [<step-2]
	# [Account B] Connect to WebSocket for bonded flow [>step-3]
	async with connect(WS_URL) as websocket:
		response = json.loads(await websocket.recv())
		uid = response['uid']
		print(f'[Account B] Connected to {WS_URL} with uid {uid}')

		# Subscribe to bonded transaction channels
		channels = [
			f'partialAdded/{account_b_address}',
			f'partialRemoved/{account_b_address}',
			f'cosignature/{account_b_address}',
			f'unconfirmedAdded/{account_b_address}',
			f'unconfirmedRemoved/{account_b_address}',
			f'confirmedAdded/{account_b_address}',
			f'status/{account_b_address}',
		]
		for channel in channels:
			await websocket.send(json.dumps(
				{'uid': uid, 'subscribe': channel}
			))
			name = channel.split('/')[0]
			print(f'[Account B] Subscribed to {name} channel')
# [<step-3]
		# [Account A] Announce bonded aggregate [>step-4]
		request = urllib.request.Request(
			f'{NODE_URL}/transactions/partial',
			data=bonded_payload.encode(),
			headers={'Content-Type': 'application/json'},
			method='PUT'
		)
		with urllib.request.urlopen(request) as resp:
			resp.read()
		print(f'[Account A] Announced bonded {str(bonded_hash)[:16]}...')
		# [<step-4]
		# [Account B] Listen for bonded transaction flow [>step-5]
		async for raw_message in websocket:
			message = json.loads(raw_message)
			topic = message['topic']
			name = topic.split('/')[0]

			if name == 'cosignature':
				signer = message['data']['signerPublicKey']
				print(f'cosignature: signer={signer[:16]}...')

			elif name == 'status':
				status_hash = message['data']['hash']
				print(f'status: hash={status_hash[:16]}...')
				if status_hash == str(bonded_hash):
					raise RuntimeError(
						'Transaction failed: ' + message['data']['code'])

			elif name == 'partialAdded':
				message_hash = message['data']['meta']['hash']
				print(f'partialAdded: hash={message_hash[:16]}...')
				if message_hash == str(bonded_hash):
					cosignature = facade.cosign_transaction_hash(
						account_b_key_pair, bonded_hash, True)
					cosignature_payload = json.dumps({
						'version': str(cosignature.version),
						'signerPublicKey': str(
							cosignature.signer_public_key),
						'signature': str(cosignature.signature),
						'parentHash': str(cosignature.parent_hash)
					})
					cosignature_request = (
						urllib.request.Request(
							f'{NODE_URL}/transactions/cosignature',
							data=(cosignature_payload.encode()),
							headers={'Content-Type': 'application/json'},
							method='PUT'))
					with urllib.request.urlopen(
						cosignature_request
					) as resp:
						resp.read()
					print('[Account B] Submitted cosignature')

			elif name == 'confirmedAdded':
				message_hash = message['data']['meta']['hash']
				print(f'confirmedAdded: hash={message_hash[:16]}...')
				if message_hash == str(bonded_hash):
					print('Transaction '
						f'{str(bonded_hash)[:16]}... confirmed')
					break

			else:
				message_hash = message['data']['meta']['hash']
				print(f'{name}: hash={message_hash[:16]}...')
# [<step-5]
		# Unsubscribe before closing [>step-6]
		for channel in channels:
			await websocket.send(json.dumps({
				'uid': uid,
				'unsubscribe': channel
			}))
		print('[Account B] Unsubscribed from all channels')
		# [<step-6]

try:
	asyncio.run(main())
except Exception as error:
	print(error)
