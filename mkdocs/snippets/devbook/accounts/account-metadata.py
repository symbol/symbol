import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.Metadata import (
	metadata_generate_key,
	metadata_update_value
)
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
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
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')


# Helper function to wait for transaction confirmation
def wait_for_confirmation(transaction_hash, label):
	print(f'Waiting for {label} confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			url = f'{NODE_URL}/transactionStatus/{transaction_hash}'
			with urllib.request.urlopen(url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
				if status['group'] == 'confirmed':
					print(f'{label} confirmed in {attempt} seconds')
					return
				if status['group'] == 'failed':
					raise Exception(f'{label} failed: {status["code"]}')
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	raise Exception(f'{label} not confirmed after 60 seconds')

# [>step-1]
SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')
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
	# --- ADDING NEW METADATA ---
	print('\n--- Adding new metadata ---')

	# Define metadata key and value [>step-3]
	key_string = f'username_{int(time.time())}'
	scoped_metadata_key = metadata_generate_key(key_string)
	metadata_value = 'alice'.encode('utf8')
	# [<step-3]
	# Create the embedded metadata transaction [>step-4]
	embedded_transaction = facade.transaction_factory.create_embedded({
		'type': 'account_metadata_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'target_address': signer_address,
		'scoped_metadata_key': scoped_metadata_key,
		# When creating new metadata, value_size_delta
		# equals the value length
		'value_size_delta': len(metadata_value),
		'value': metadata_value
	})
	print('Created embedded metadata transaction:')
	print(json.dumps(embedded_transaction.to_json(), indent=2))
	# [<step-4]
	# Build the aggregate transaction [>step-5]
	embedded_transactions = [embedded_transaction]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	# [<step-5]
	# Sign and generate final payload [>step-6]
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)

	# Announce and wait for confirmation
	transaction_hash = facade.hash_transaction(transaction)
	print(f'Built aggregate transaction with hash: {transaction_hash}')
	announce_transaction(json_payload, 'aggregate transaction')
	wait_for_confirmation(transaction_hash, 'aggregate transaction')
	# [<step-6]
	# --- MODIFYING EXISTING METADATA ---
	print('\n--- Modifying existing metadata ---')

	# Fetch current metadata value from network [>step-7]
	metadata_path = (
		f'/metadata?sourceAddress={signer_address}'
		f'&targetAddress={signer_address}'
		f'&scopedMetadataKey={scoped_metadata_key:016X}'
		'&metadataType=0'
	)
	print(f'Fetching current metadata from {metadata_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{metadata_path}') as response:
		response_json = json.loads(response.read().decode())

	# Get the metadata entry
	if not response_json['data']:
		raise Exception('Metadata entry not found')
	metadata_entry = response_json['data'][0]['metadataEntry']
	current_value = bytes.fromhex(metadata_entry['value'])
	print(f'  Current value: {current_value.decode("utf8")}')
	# [<step-7]
	# XOR the current and new values [>step-8]
	new_value = 'bob'.encode('utf8')
	update_value = metadata_update_value(current_value, new_value)

	# Create the update transaction with XOR'd value
	embedded_update = facade.transaction_factory.create_embedded({
		'type': 'account_metadata_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'target_address': signer_address,
		'scoped_metadata_key': scoped_metadata_key,
		# value_size_delta is the difference in length
		# (can be negative)
		'value_size_delta': len(new_value) - len(current_value),
		'value': update_value
	})
	# [<step-8]
	# Build the aggregate for the update [>step-9]
	embedded_transactions = [embedded_update]
	update_transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	update_transaction.fee = Amount(fee_mult * update_transaction.size)

	# Sign and announce the update
	signature = facade.sign_transaction(
		signer_key_pair, update_transaction)
	json_payload = facade.transaction_factory.attach_signature(
		update_transaction, signature)

	# Announce and wait for confirmation
	update_hash = facade.hash_transaction(update_transaction)
	print(f'Built aggregate transaction with hash: {update_hash}')
	announce_transaction(json_payload, 'aggregate transaction')
	wait_for_confirmation(update_hash, 'aggregate transaction')
	# [<step-9]
except Exception as e:
	print(e)
