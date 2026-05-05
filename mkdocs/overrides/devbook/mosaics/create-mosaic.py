import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import generate_mosaic_id
from symbolchain.sc import Amount

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')


# Helper function to announce a transaction
def announce_transaction(payload, label):
	print(f'Announcing {label} to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')


# Helper function to wait for transaction confirmation
def wait_for_confirmation(transaction_hash, label):
	print(f'Waiting for {label} confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			url = f'{NODE_URL}/transactionStatus/{transaction_hash}'
			with urllib.request.urlopen(url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
				if status['group'] == 'confirmed':
					print(f'{label} confirmed in {attempt} seconds')
					return
				if status['group'] == 'failed':
					raise Exception(f'{label} failed: {status["code"]}')
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	raise Exception(f'{label} not confirmed after 60 seconds')

# [>step-1]
SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(
	PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')
# [<step-1]
try:
	# Fetch current network time [>step-2]
	time_path = '/node/time'
	print(f'Fetching current network time from {time_path}')
	with urllib.request.urlopen(f'{NODE_URL}{time_path}') as response:
		response_json = json.loads(response.read().decode())
		receive_timestamp = (
			response_json['communicationTimestamps']['receiveTimestamp'])
		timestamp = NetworkTimestamp(int(receive_timestamp))
		print(f'  Network time: {timestamp.timestamp} ms since nemesis')

	# Fetch recommended fees
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(f'{NODE_URL}{fee_path}') as response:
		response_json = json.loads(response.read().decode())
		median_mult = response_json['medianFeeMultiplier']
		minimum_mult = response_json['minFeeMultiplier']
		fee_mult = max(median_mult, minimum_mult)
		print(f'  Fee multiplier: {fee_mult}')
	# [<step-2]
	# --- CREATING MOSAIC DEFINITION ---
	print('\n--- Creating mosaic definition ---')
	# [>step-3]
	nonce = int(time.time()) & 0xFFFFFFFF
	print(f'Mosaic nonce: {nonce}')

	definition_tx = facade.transaction_factory.create({
		'type': 'mosaic_definition_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'duration': 0,
		'divisibility': 2,
		'nonce': nonce,
		'flags': 'transferable restrictable'
	})
	definition_tx.fee = Amount(fee_mult * definition_tx.size)

	mosaic_id = generate_mosaic_id(signer_address, nonce)
	print(f'Mosaic ID: {mosaic_id} ({hex(mosaic_id)})')
	# [<step-3]
	# Sign and generate final payload [>step-4]
	signature = facade.sign_transaction(signer_key_pair, definition_tx)
	json_payload = facade.transaction_factory.attach_signature(
			definition_tx, signature)
	print('Built mosaic definition transaction:')
	print(json.dumps(definition_tx.to_json(), indent=2))

	# Announce and wait for confirmation
	definition_hash = facade.hash_transaction(definition_tx)
	print(f'Transaction hash: {definition_hash}')
	announce_transaction(json_payload, 'mosaic definition')
	wait_for_confirmation(definition_hash, 'mosaic definition')
	# [<step-4]
	# --- INCREASING MOSAIC SUPPLY ---
	print('\n--- Increasing mosaic supply ---')
	# [>step-5]
	supply_tx = facade.transaction_factory.create({
		'type': 'mosaic_supply_change_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'mosaic_id': mosaic_id,
		'action': 'increase',
		'delta': 100_00
	})
	supply_tx.fee = Amount(fee_mult * supply_tx.size)
	# [<step-5]
	# Sign and generate final payload [>step-6]
	signature = facade.sign_transaction(signer_key_pair, supply_tx)
	json_payload = facade.transaction_factory.attach_signature(
			supply_tx, signature)
	print(
		'Built mosaic supply change transaction:')
	print(json.dumps(supply_tx.to_json(), indent=2))

	# Announce and wait for confirmation
	supply_hash = facade.hash_transaction(supply_tx)
	print(f'Transaction hash: {supply_hash}')
	announce_transaction(json_payload, 'mosaic supply change')
	wait_for_confirmation(supply_hash, 'mosaic supply change')
	# [<step-6]
	# --- VERIFYING MOSAIC ---
	print('\n--- Verifying mosaic ---')
	# [>step-7]
	mosaic_id_hex = f'{mosaic_id:016x}'
	mosaic_path = f'/mosaics/{mosaic_id_hex}'
	print(f'Fetching mosaic information from {mosaic_path}')
	with urllib.request.urlopen(f'{NODE_URL}{mosaic_path}') as response:
		response_json = json.loads(response.read().decode())
		mosaic_info = response_json['mosaic']
		print('Mosaic information:')
		print(f'  Mosaic ID: {mosaic_info["id"]}')
		print(f'  Supply: {mosaic_info["supply"]}')
		print(f'  Flags: {mosaic_info["flags"]}')
		print(f'  Divisibility: {mosaic_info["divisibility"]}')
		print(f'  Duration: {mosaic_info["duration"]}')
	# [<step-7]
except Exception as e:
	print(e)
