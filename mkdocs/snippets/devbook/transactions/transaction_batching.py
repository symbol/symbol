import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import Address

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


SIGNER_PRIVATE_KEY = os.getenv(  # [>step-1]
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(
	PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer public key: {signer_key_pair.public_key}')
print(f'Signer address: {signer_address}')

RECIPIENT_1 = os.getenv(
	'RECIPIENT_1', 'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI')
RECIPIENT_2 = os.getenv(
	'RECIPIENT_2', 'TCD4NC5VIE2EEB3BCV5JRLBNJXYDW5Q5JK547MI')
recipient1_hex = Address(RECIPIENT_1).bytes.hex().upper()
recipient2_hex = Address(RECIPIENT_2).bytes.hex().upper()
print(f'Recipient 1: {RECIPIENT_1} ({recipient1_hex})')
print(f'Recipient 2: {RECIPIENT_2} ({recipient2_hex})')  # [<step-1]

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
	# Embedded tx 1: Send 5 XYM to Recipient 1 [>step-3]
	xym_mosaic_id = generate_mosaic_alias_id('symbol.xym')
	embedded_tx_1 = facade.create_embedded_transaction_from_descriptor(
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': Address(RECIPIENT_1),
			'mosaics': [{
				'mosaic_id': xym_mosaic_id,
				'amount': 5_000_000  # 5 XYM
			}]
		},
		signer_key_pair.public_key)

	# Embedded tx 2: Send 3 XYM to Recipient 2
	embedded_tx_2 = facade.create_embedded_transaction_from_descriptor(
		{
			'type': 'transfer_transaction_v1',
			'recipient_address': Address(RECIPIENT_2),
			'mosaics': [{
				'mosaic_id': xym_mosaic_id,
				'amount': 3_000_000  # 3 XYM
			}]
		},
		signer_key_pair.public_key)  # [<step-3]

	# Build the aggregate transaction [>step-4]
	embedded_transactions = [embedded_tx_1, embedded_tx_2]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash':
				facade.hash_embedded_transactions(embedded_transactions),
			'transactions': embedded_transactions
		},
		signer_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)
	print('Built aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))  # [<step-4]

	# Sign transaction and generate final payload [>step-5]
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)

	# Announce the transaction
	announce_transaction(json_payload, 'transaction')
	# [<step-5]
	# Wait for confirmation [>step-6]
	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')
	wait_for_confirmation(transaction_hash, 'transaction')
	# [<step-6]
except Exception as e:
	print(e)
