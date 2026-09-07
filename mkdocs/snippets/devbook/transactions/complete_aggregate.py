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


# Account A (initiates the aggregate tx and sends XYM to Account B) [>step-1]
ACCOUNT_A_PRIVATE_KEY = os.getenv(
	'ACCOUNT_A_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
account_a_key_pair = SymbolFacade.KeyPair(
	PrivateKey(ACCOUNT_A_PRIVATE_KEY))

# Account B (sends custom mosaic to Account A)
ACCOUNT_B_PRIVATE_KEY = os.getenv(
	'ACCOUNT_B_PRIVATE_KEY',
	'1111111111111111111111111111111111111111111111111111111111111111')
account_b_key_pair = SymbolFacade.KeyPair(
	PrivateKey(ACCOUNT_B_PRIVATE_KEY))

facade = SymbolFacade('testnet')
account_a_address = facade.network.public_key_to_address(
	account_a_key_pair.public_key)
account_b_address = facade.network.public_key_to_address(
	account_b_key_pair.public_key)
print(f'Account A: {account_a_address}')
print(f'Account B: {account_b_address}')  # [<step-1]

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

	# Embedded tx 1: Account A transfers 10 XYM to Account B [>step-3]
	embedded_transaction_1 = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': account_b_address,
				'mosaics': [{
					'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
					'amount': 10_000_000  # 10 XYM
				}]
			},
			account_a_key_pair.public_key))

	# Embedded tx 2: Account B transfers 1 custom mosaic to Account A
	custom_mosaic_id = 0x6D1314BE751B62C2
	embedded_transaction_2 = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': account_a_address,
				'mosaics': [{
					'mosaic_id': custom_mosaic_id,
					'amount': 1  # 1 custom mosaic
				}]
			},
			account_b_key_pair.public_key))  # [<step-3]

	# Build the aggregate transaction [>step-4]
	embedded_transactions = [
		embedded_transaction_1, embedded_transaction_2]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				embedded_transactions),
			'transactions': embedded_transactions
		},
		account_a_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60,
		1)
	print('Built aggregate transaction without signatures:')
	print(json.dumps(transaction.to_json(), indent=2))  # [<step-4]

	# --- ACCOUNT A (Initiator) --- [>step-5]
	print('[Account A] Signing the aggregate...')
	signature_a = facade.sign_transaction(
		account_a_key_pair, transaction)
	transaction_payload = facade.transaction_factory.attach_signature(
		transaction, signature_a)
	payload_formatted = json.dumps(
		json.loads(transaction_payload), indent=2)
	print(f'[Account A] Payload ready to share:\n{payload_formatted}')

	# --- OFF-CHAIN COORDINATION ---
	# Account A sends the payload to Account B
	shared_payload = transaction_payload
	print('[Account A] ==> Payload sent to Account B (offchain)')
	# [<step-5]
	# --- ACCOUNT B (Cosignatory) --- [>step-6]
	received_transaction = facade.transaction_factory.deserialize(
		bytes.fromhex(json.loads(shared_payload)['payload']))

	print('[Account B] Cosigning...')
	cosignature_b = facade.cosign_transaction(
		account_b_key_pair, received_transaction)
	cosignature_formatted = json.dumps(cosignature_b.to_json(), indent=2)
	print(f'[Account B] Cosignature created: {cosignature_formatted}')

	# --- OFF-CHAIN COORDINATION ---
	# Account B sends the cosignature back to Account A
	shared_cosignature = cosignature_b
	print(
		'[Account B] <== Cosignature sent back to Account A (offchain)')
	# [<step-6]
	# --- ACCOUNT A (Initiator) --- [>step-7]
	# Add cosignature to the transaction and rebuild payload
	transaction.cosignatures.append(shared_cosignature)
	transaction_payload = facade.transaction_factory.to_json(transaction)
	json_payload = transaction_payload
	print('[Account A] Ready to announce')  # [<step-7]

	# Announce the transaction [>step-8]
	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')
	announce_transaction(json_payload, 'transaction')  # [<step-8]

	# Wait for confirmation [>step-9]
	wait_for_confirmation(transaction_hash, 'transaction')
	# [<step-9]
except Exception as e:
	print(e)
