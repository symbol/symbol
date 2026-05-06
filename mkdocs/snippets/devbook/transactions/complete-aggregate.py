import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

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
print(f'Account B: {account_b_address}')
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
	# Embedded tx 1: Account A transfers 10 XYM to Account B [>step-3]
	embedded_transaction_1 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': account_a_key_pair.public_key,
		'recipient_address': account_b_address,
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 10_000_000  # 10 XYM (divisibility = 6)
		}]
	})

	## Embedded tx 2: Account B transfers 1 custom mosaic to Account A
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
	# [<step-3]
	# Build the aggregate transaction [>step-4]
	embedded_transactions = [
		embedded_transaction_1, embedded_transaction_2]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		'signer_public_key': account_a_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	# Reserve space for one cosignature (104 bytes)
	# and calculate fee for the final transaction size
	transaction.fee = Amount(fee_mult * (transaction.size + 104))
	print('Built aggregate transaction without signatures:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-4]
	# --- ACCOUNT A (Initiator) --- [>step-5]
	print('[Account A] Signing the aggregate...')
	signature_a = facade.sign_transaction(account_a_key_pair, transaction)
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
	print('[Account B] <== Cosignature sent back to Account A (offchain)')
	# [<step-6]
	# --- ACCOUNT A (Initiator) --- [>step-7]
	# Add cosignature to the transaction and rebuild payload
	transaction.cosignatures.append(shared_cosignature)
	transaction_payload = facade.transaction_factory.attach_signature(
		transaction, signature_a)
	json_payload = transaction_payload
	print('[Account A] Ready to announce')
	# [<step-7]
	# Announce the transaction [>step-8]
	announce_path = '/transactions'
	print(f'Announcing transaction to {announce_path}')
	announce_request = urllib.request.Request(
		f'{NODE_URL}{announce_path}',
		data=json_payload.encode(),
		headers={ 'Content-Type': 'application/json' },
		method='PUT'
	)
	with urllib.request.urlopen(announce_request) as response:
		print(f'  Response: {response.read().decode()}')

	# Compute hash of final transaction (with cosignatures)
	transaction_hash = facade.hash_transaction(transaction)
	# [<step-8]
	# Wait for confirmation [>step-9]
	status_path = f'/transactionStatus/{transaction_hash}'
	print(f'Waiting for confirmation from {status_path}')
	for attempt in range(60):
		time.sleep(1)
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
	else:
		print('Confirmation took too long.')
	# [<step-9]
except Exception as e:
	print(e)
