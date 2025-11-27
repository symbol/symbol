import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import Hash256, PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.environ.get(
	'NODE_URL', 'https://001-sai-dual.symboltest.net:3001')
print(f'Using node {NODE_URL}')

# Helper function to announce transaction
def announce_transaction(payload, endpoint, label):
	print(f'Announcing {label} to {endpoint}')
	request = urllib.request.Request(
		f'{NODE_URL}{endpoint}',
		data=payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')

# Helper function to wait for transaction status
def wait_for_status(hash_value, expected_status, label):
	print(f'Waiting for {label} to reach {expected_status} status...')
	attempts = 0
	max_attempts = 60

	while attempts < max_attempts:
		try:
			url = f'{NODE_URL}/transactionStatus/{hash_value}'
			with urllib.request.urlopen(url) as response:
				status = json.loads(response.read().decode())

				print(f'  Transaction status: {status["group"]}')

				if status['group'] == 'failed':
					raise Exception(f'{label} failed: {status["code"]}')

				if status['group'] == expected_status:
					print(f'{label} {expected_status} ' +
					f'in {attempts} seconds')
					return

		except urllib.error.HTTPError as e:
			if e.code != 404:
				raise
			# Transaction status not yet available

		attempts += 1
		time.sleep(1)

	raise Exception(
		f'{label} not {expected_status} after {max_attempts} attempts'
	)

# Account A (initiates the aggregate tx and sends XYM to Account B)
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
print(f'Account B: {account_b_address}')

try:
	# Fetch current network time
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

	# Embedded tx 1: Account A transfers 10 XYM to Account B
	embedded_transaction_1 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': account_a_key_pair.public_key,
		'recipient_address': account_b_address,
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 10_000_000  # 10 XYM (divisibility = 6)
		}]
	})

	# Embedded tx 2: Account B transfers 1 custom mosaic to Account A
	custom_mosaic_id = 0x6D1314BE751B62C2
	embedded_transaction_2 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': account_b_key_pair.public_key,
		'recipient_address': account_a_address,
		'mosaics': [{
			'mosaic_id': custom_mosaic_id,
			'amount': 1  # 1 custom mosaic (divisibility = 0)
		}]
	})

	# Build the bonded aggregate transaction
	embedded_transactions = [
		embedded_transaction_1, embedded_transaction_2]
	bonded_transaction = facade.transaction_factory.create({
		'type': 'aggregate_bonded_transaction_v3',
		'signer_public_key': account_a_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	# Reserve space for one cosignature (104 bytes)
	# and calculate fee for the final transaction size
	bonded_transaction.fee = Amount(
		fee_mult * (bonded_transaction.size + 104))
	print('Built aggregate without signatures:')
	print(json.dumps(bonded_transaction.to_json(), indent=2))

	# --- ACCOUNT A (Initiator) ---
	# Sign the bonded aggregate transaction
	print('[Account A] Signing the bonded aggregate...')
	bonded_signature = facade.sign_transaction(
		account_a_key_pair, bonded_transaction)
	bonded_json_payload = facade.transaction_factory.attach_signature(
		bonded_transaction, bonded_signature)
	bonded_hash = facade.hash_transaction(bonded_transaction)
	print(f'Bonded aggregate transaction hash: {bonded_hash}')

	# Create hash lock transaction
	print('Creating hash lock transaction...')
	hash_lock = facade.transaction_factory.create({
		'type': 'hash_lock_transaction_v1',
		'signer_public_key': account_a_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'mosaic': {
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 10_000_000  # 10 XYM deposit
		},
		'duration': 100,  # Lock duration in blocks
		'hash': bonded_hash
	})
	hash_lock.fee = Amount(fee_mult * hash_lock.size)

	# Sign hash lock
	print('[Account A] Signing the hash lock...')
	hash_lock_signature = facade.sign_transaction(
		account_a_key_pair, hash_lock)
	hash_lock_payload = facade.transaction_factory.attach_signature(
		hash_lock, hash_lock_signature)
	hash_lock_hash = facade.hash_transaction(hash_lock)
	print(f'Hash lock transaction hash: {hash_lock_hash}')

	# Announce hash lock and wait for confirmation
	announce_transaction(hash_lock_payload, '/transactions', 'Hash lock')
	wait_for_status(hash_lock_hash, 'confirmed', 'Hash lock')

	# Announce bonded aggregate and wait for partial status
	announce_transaction(
		bonded_json_payload, '/transactions/partial',
		'Bonded aggregate transaction'
	)
	wait_for_status(
		bonded_hash, 'partial',
		'Bonded aggregate transaction'
	)

	# --- ACCOUNT B (Cosigner) ---
	# Retrieves partial transactions waiting for signature
	partial_path = f'/transactions/partial?address={account_b_address}'
	print(
		'[Account B] Checking for partial transactions from '
		'/transactions/partial'
	)
	with urllib.request.urlopen(f'{NODE_URL}{partial_path}') as response:
		partial_txs = json.loads(response.read().decode())
		if not partial_txs['data']:
			raise Exception('No partial transactions found')

	print(f'Found {len(partial_txs["data"])} partial transaction(s)')

	# Find the transaction matching the expected hash
	found = any(
		tx['meta']['hash'] == str(bonded_hash)
		for tx in partial_txs['data']
	)
	if not found:
		raise Exception(
			f'Expected transaction {bonded_hash} not found in '
			f'partial transactions')
	print(f'Found matching transaction: {bonded_hash}')

	# Fetch full transaction details using the hash
	detail_path = f'/transactions/partial/{bonded_hash}'
	with urllib.request.urlopen(f'{NODE_URL}{detail_path}') as response:
		partial_tx_json = json.loads(response.read().decode())

	# Verify transaction content before cosigning
	tx_data = partial_tx_json['transaction']
	print(
		f'[Account B] Verifying transaction: '
		f'{len(tx_data["transactions"])} embedded transactions'
	)

	# Submit Account B's cosignature using the transaction hash
	cosignature_path = '/transactions/cosignature'
	print('[Account B] Cosigning the bonded aggregate...')
	cosignature = facade.cosign_transaction_hash(
		account_b_key_pair, bonded_hash, True)
	cosignature_payload = json.dumps({
		'version': str(cosignature.version),
		'signerPublicKey': str(cosignature.signer_public_key),
		'signature': str(cosignature.signature),
		'parentHash': str(cosignature.parent_hash)
	})

	# Announce cosignature
	announce_transaction(
		cosignature_payload, cosignature_path, 'cosignature'
	)

	# Wait for final confirmation
	wait_for_status(
		bonded_hash, 'confirmed',
		'Bonded aggregate transaction'
	)

except Exception as e:
	print(e)
