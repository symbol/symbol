import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id

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


MULTISIG_PRIVATE_KEY = os.getenv(  # [>step-1]
	'MULTISIG_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000001')
multisig_key_pair = SymbolFacade.KeyPair(
	PrivateKey(MULTISIG_PRIVATE_KEY))
print(f'Multisig public key: {multisig_key_pair.public_key}')
COSIGNATORY0_PRIVATE_KEY = os.getenv(
	'COSIGNATORY0_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000002')
cosignatory_key_pair = SymbolFacade.KeyPair(
	PrivateKey(COSIGNATORY0_PRIVATE_KEY))
print(f'Cosignatory public key: {cosignatory_key_pair.public_key}')
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
		print(f'  Fee multiplier: {fee_multiplier}')  # [<step-2]

	# Build the embedded transfer transaction [>step-3]
	transfer_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address':
					facade.network.public_key_to_address(
						multisig_key_pair.public_key),
				'mosaics': [{
					'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
					'amount': 1_000_000  # 1 XYM
				}]
			},
			multisig_key_pair.public_key))  # [<step-3]

	# Build the wrapper aggregate transaction [>step-4]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				[transfer_transaction]),
			'transactions': [transfer_transaction]
		},
		cosignatory_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)  # [<step-4]

	# [>step-5]
	# Sign the aggregate transaction using the cosignatory's signature
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(cosignatory_key_pair, transaction))
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))  # [<step-5]

	# Announce the transaction [>step-6]
	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')
	announce_transaction(json_payload, 'transaction')

	# Wait for confirmation
	wait_for_confirmation(transaction_hash, 'transaction')  # [<step-6]

except urllib.error.URLError as e:
	print(e.reason)
