import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = 'https://001-sai-dual.symboltest.net:3001'
print(f'Using node {NODE_URL}')

# Account A (initiates the aggregate and sends XYM)
ACCOUNT_A_PRIVATE_KEY = os.getenv(
	'ACCOUNT_A_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
account_a_key_pair = SymbolFacade.KeyPair(
	PrivateKey(ACCOUNT_A_PRIVATE_KEY))

# Account B (sends custom mosaic back to Account A)
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

	# Account A sends 10 XYM to Account B
	embedded_transaction_1 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': account_a_key_pair.public_key,
		'recipient_address': account_b_address,
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 10_000_000  # 10 XYM (divisibility = 6)
		}]
	})

	# Account B sends 1 custom mosaic to Account A
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

	# Build the aggregate bonded transaction
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
	# Reserve space for one cosignature (104 bytes each)
	# and calculate fee for the final transaction size
	bonded_transaction.fee = Amount(
		fee_mult * (bonded_transaction.size + 104))
	print('Built aggregate bonded transaction:')
	print(json.dumps(bonded_transaction.to_json(), indent=2))

	# Sign the bonded transaction
	bonded_signature = facade.sign_transaction(
		account_a_key_pair, bonded_transaction)
	bonded_json_payload = facade.transaction_factory.attach_signature(
		bonded_transaction, bonded_signature)
	bonded_hash = facade.hash_transaction(bonded_transaction)
	print(f'Bonded transaction hash: {bonded_hash}')

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

	# Sign and announce hash lock
	hash_lock_signature = facade.sign_transaction(
		account_a_key_pair, hash_lock)
	hash_lock_payload = facade.transaction_factory.attach_signature(
		hash_lock, hash_lock_signature)
	hash_lock_hash = facade.hash_transaction(hash_lock)
	print(f'Hash lock transaction hash: {hash_lock_hash}')

	announce_path = '/transactions'
	print(f'Announcing hash lock to {announce_path}')
	announce_request = urllib.request.Request(
		f'{NODE_URL}{announce_path}',
		data=hash_lock_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(announce_request) as response:
		print(f'  Response: {response.read().decode()}')

	# Wait for hash lock confirmation
	print(f'Waiting for hash lock confirmation...')
	status_path = f'/transactionStatus/{hash_lock_hash}'
	for attempt in range(60):
		time.sleep(1)
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				if status['group'] == 'confirmed':
					print(f'  Hash lock confirmed')
					break
		except urllib.error.HTTPError as e:
			print(f'  Hash lock status: unknown | Cause: ({e.msg})')

	# Announce bonded transaction
	partial_path = '/transactions/partial'
	print(f'Announcing bonded transaction to {partial_path}')
	partial_request = urllib.request.Request(
		f'{NODE_URL}{partial_path}',
		data=bonded_json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(partial_request) as response:
		print(f'  Response: {response.read().decode()}')

	# Wait for transaction to reach partial status
	print('Waiting for bonded transaction to reach partial status...')
	status_path = f'/transactionStatus/{bonded_hash}'
	for attempt in range(60):
		time.sleep(1)
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				if status['group'] == 'partial':
					print(
						'  Transaction is partial, '
						'ready for cosignatures'
					)
					break
		except urllib.error.HTTPError as e:
			print(f'  Transaction status: unknown | Cause: ({e.msg})')

	# Submit Account B's cosignature
	print('Submitting Account B\'s cosignature...')
	cosignature_path = '/transactions/cosignature'
	cosignature = facade.cosign_transaction(
		account_b_key_pair, bonded_transaction, True)
	cosignature_payload = json.dumps({
		'version': str(cosignature.version),
		'signerPublicKey': str(cosignature.signer_public_key),
		'signature': str(cosignature.signature),
		'parentHash': str(cosignature.parent_hash)
	})

	cosig_request = urllib.request.Request(
		f'{NODE_URL}{cosignature_path}',
		data=cosignature_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(cosig_request) as response:
		print(f'  Cosignature from Account B: {response.read().decode()}')

	# Wait for final confirmation
	print(f'Waiting for bonded transaction confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
				if status['group'] == 'confirmed':
					print(f'Transaction confirmed in {attempt} seconds')
					break
				if status['group'] == 'failed':
					print(f'Transaction failed: {status["code"]}')
					break
		except urllib.error.HTTPError as e:
			print(f'  Transaction status: unknown | Cause: ({e.msg})')
	else:
		print('Confirmation took too long.')

except Exception as e:
	print(e)
