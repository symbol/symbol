import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_id

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
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
	with urllib.request.urlopen(request) as announce_response:
		print(f'  Response: {announce_response.read().decode()}')


# Helper function to wait for transaction confirmation
def wait_for_confirmation(tx_hash, label):
	print(f'Waiting for {label} confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			url = f'{NODE_URL}/transactionStatus/{tx_hash}'
			with urllib.request.urlopen(url) as confirm_response:
				status = json.loads(confirm_response.read().decode())
				print(f'  Transaction status: {status["group"]}')
				if status['group'] == 'confirmed':
					print(f'{label} confirmed in {attempt} seconds')
					return
				if status['group'] == 'failed':
					raise RuntimeError(
						f'{label} failed: {status["code"]}')
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	raise TimeoutError(f'{label} not confirmed after 60 seconds')


SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',  # [>step-1]
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(
	PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')
# [<step-1]
try:
	# Fetch recommended fees [>step-2]
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(f'{NODE_URL}{fee_path}') as response:
		response_json = json.loads(response.read().decode())
		median_multiplier = response_json['medianFeeMultiplier']
		minimum_multiplier = response_json['minFeeMultiplier']
		fee_multiplier = max(median_multiplier, minimum_multiplier)
		print(f'  Fee multiplier: {fee_multiplier}')
	# [<step-2]
	# Build the modification transaction [>step-3]
	MOSAIC_NONCE = int(os.getenv('MOSAIC_NONCE', '0'))
	print(f'Mosaic nonce: {MOSAIC_NONCE}')

	mosaic_id = generate_mosaic_id(signer_address, MOSAIC_NONCE)
	print(f'Mosaic ID: {mosaic_id} (0x{mosaic_id:016X})')

	modify_tx = facade.create_transaction_from_descriptor(
		{
			'type': 'mosaic_definition_transaction_v1',
			'duration': 0,
			'divisibility': 0,
			'nonce': MOSAIC_NONCE,
			'flags': 'revokable'
		},
		signer_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)
	# [<step-3]
	# Sign and generate final payload [>step-4]
	signature = facade.sign_transaction(signer_key_pair, modify_tx)
	json_payload = facade.transaction_factory.attach_signature(
		modify_tx, signature)
	print('Built mosaic modification transaction:')
	print(json.dumps(modify_tx.to_json(), indent=2))

	modify_hash = facade.hash_transaction(modify_tx)
	print(f'Transaction hash: {modify_hash}')

	# Announce transaction
	announce_transaction(json_payload, 'mosaic modification')
	# [<step-4]
	# Wait for confirmation [>step-5]
	wait_for_confirmation(modify_hash, 'mosaic modification')
	# [<step-5]
	# Retrieve the mosaic [>step-6]
	mosaic_id_hex = f'{mosaic_id:016X}'
	mosaic_path = f'/mosaics/{mosaic_id_hex}'
	print(f'Fetching mosaic information from {mosaic_path}')
	with urllib.request.urlopen(f'{NODE_URL}{mosaic_path}') as response:
		response_json = json.loads(response.read().decode())
		mosaic_info = response_json['mosaic']
		print('Mosaic information:')
		print(f'  Mosaic ID: {mosaic_info["id"]}')
		print(f'  Supply: {mosaic_info["supply"]}')
		print(f'  Divisibility: {mosaic_info["divisibility"]}')
		print(f'  Flags: {mosaic_info["flags"]}')
		print(f'  Duration: {mosaic_info["duration"]}')
	# [<step-6]
except Exception as e:
	print(e)
