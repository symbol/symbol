import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')

print(f'Using node {NODE_URL}')
# [>step-1]
SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))
# [<step-1]
facade = SymbolFacade('testnet')

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
	# Build the transaction [>step-3]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'transfer_transaction_v1',
			'recipient_address':
				facade.network.public_key_to_address(
					signer_key_pair.public_key),
			'mosaics': [{
				'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
				'amount': 1_000_000  # 1 XYM
			}]
		},
		signer_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)
	# [<step-3]
	# Sign transaction and generate final payload [>step-4]
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-4]
	# Announce the transaction [>step-5]
	announce_path = '/transactions'
	print(f'Announcing transaction to {announce_path}')
	announce_request = urllib.request.Request(
		f'{NODE_URL}{announce_path}',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(announce_request) as response:
		print(f'  Response: {response.read().decode()}')
	# [<step-5]
	# Wait for confirmation [>step-6]
	status_path = (
		f'/transactionStatus/{facade.hash_transaction(transaction)}')
	print(f'Waiting for confirmation from {status_path}')
	for attempt in range(60):
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status['group']}')
				if status['group'] == 'confirmed':
					print(f'Transaction confirmed in {attempt} seconds')
					break
				if status['group'] == 'failed':
					print(f'Transaction failed: {status['code']}')
					break
		except urllib.error.HTTPError as e:
			print(f'  Transaction status: unknown | Cause: ({e.msg})')
		time.sleep(1)
	else:
		print('Confirmation took too long.')
	# [<step-6]
except urllib.error.URLError as e:
	print(e.reason)
