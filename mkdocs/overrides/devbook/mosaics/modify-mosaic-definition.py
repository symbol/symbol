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

SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(
	PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

try:
	# Fetch current network time
	time_path = '/node/time'
	print(f'Fetching current network time from {time_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{time_path}') as response:
		response_json = json.loads(response.read().decode())
		receive_timestamp = (
			response_json['communicationTimestamps']['receiveTimestamp'])
		timestamp = NetworkTimestamp(int(receive_timestamp))
		print(f'  Network time: {timestamp.timestamp} ms since nemesis')

	# Fetch recommended fees
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{fee_path}') as response:
		response_json = json.loads(response.read().decode())
		median_mult = response_json['medianFeeMultiplier']
		minimum_mult = response_json['minFeeMultiplier']
		fee_mult = max(median_mult, minimum_mult)
		print(f'  Fee multiplier: {fee_mult}')

	# Build the modification transaction
	MOSAIC_NONCE = int(os.getenv('MOSAIC_NONCE', '0'))
	print(f'Mosaic nonce: {MOSAIC_NONCE}')

	mosaic_id = generate_mosaic_id(signer_address, MOSAIC_NONCE)
	print(f'Mosaic ID: {mosaic_id} ({hex(mosaic_id)})')

	modify_tx = facade.transaction_factory.create({
		'type': 'mosaic_definition_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'duration': 0,
		'divisibility': 0,
		'nonce': MOSAIC_NONCE,
		'flags': 'revokable'
	})
	modify_tx.fee = Amount(fee_mult * modify_tx.size)

	# Sign and generate final payload
	signature = facade.sign_transaction(signer_key_pair, modify_tx)
	json_payload = facade.transaction_factory.attach_signature(
		modify_tx, signature)
	print('Built mosaic modification transaction:')
	print(json.dumps(modify_tx.to_json(), indent=2))

	modify_hash = facade.hash_transaction(modify_tx)
	print(f'Transaction hash: {modify_hash}')

	# Announce transaction
	print('Announcing mosaic modification to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')

	# Wait for confirmation
	print('Waiting for mosaic modification confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			status_url = (
				f'{NODE_URL}/transactionStatus/{modify_hash}')
			with urllib.request.urlopen(status_url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
			if status['group'] == 'confirmed':
				print('Mosaic modification confirmed in',
					attempt, 'seconds')
				break
			if status['group'] == 'failed':
				raise Exception(
					'Mosaic modification failed:', status['code'])
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')

	# Retrieve the mosaic
	mosaic_id_hex = f'{mosaic_id:x}'
	mosaic_path = f'/mosaics/{mosaic_id_hex}'
	print(f'Fetching mosaic information from {mosaic_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{mosaic_path}') as response:
		response_json = json.loads(response.read().decode())
		mosaic_info = response_json['mosaic']
		print('Mosaic information:')
		print(f'  Mosaic ID: {mosaic_info["id"]}')
		print(f'  Supply: {mosaic_info["supply"]}')
		print(f'  Divisibility: {mosaic_info["divisibility"]}')
		print(f'  Flags: {mosaic_info["flags"]}')
		print(f'  Duration: {mosaic_info["duration"]}')

except Exception as e:
	print(e)
