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


# Helper function to announce a transaction [>step-5]
def announce_transaction(payload, label):
	print(f'Announcing {label} to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as announce_response:
		print(f'  Response: {announce_response.read().decode()}')  # [<step-5]


# Helper function to wait for transaction confirmation [>step-6]
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
	raise TimeoutError(f'{label} not confirmed after 60 seconds')  # [<step-6]


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
	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')
	announce_transaction(json_payload, 'transaction')
	wait_for_confirmation(transaction_hash, 'transaction')
except urllib.error.URLError as e:
	print(e.reason)
