import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import (
	generate_mosaic_alias_id,
	generate_namespace_path
)
from symbolchain.symbol.Network import NetworkTimestamp

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
				print(f"  Transaction status: {status['group']}")
				if status['group'] == 'confirmed':
					print(f'{label} confirmed in {attempt} seconds')
					return
				if status['group'] == 'failed':
					raise RuntimeError(f"{label} failed: {status['code']}")
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	raise TimeoutError(f'{label} not confirmed after 60 seconds')


SIGNER_PRIVATE_KEY = os.getenv(  # [>step-1]
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')  # [<step-1]
# [>step-2]
namespace_name = os.getenv('NAMESPACE_NAME', 'my_namespace')
print(f'Namespace name: {namespace_name}')

namespace_id = generate_namespace_path(namespace_name)[-1]
print(f'Namespace ID: {namespace_id} ({hex(namespace_id)})')

# Target mosaic ID to link the namespace to
mosaic_id = int(os.getenv('MOSAIC_ID', '0x45C8C3733983AAC2'), 16)
print(f'Mosaic ID: {mosaic_id} ({hex(mosaic_id)})')
# [<step-2]
try:
	# Fetch current network time [>step-3]
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
		median_multiplier = response_json['medianFeeMultiplier']
		minimum_multiplier = response_json['minFeeMultiplier']
		fee_multiplier = max(median_multiplier, minimum_multiplier)
		print(f'  Fee multiplier: {fee_multiplier}')
	# [<step-3]
	# Build the alias transaction [>step-4]
	transaction = facade.transaction_factory.create({
		'type': 'mosaic_alias_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'namespace_id': namespace_id,
		'mosaic_id': mosaic_id,
		'alias_action': 'link'
	})
	transaction.fee = Amount(fee_multiplier * transaction.size)
	# [<step-4]
	# Sign transaction and generate final payload [>step-5]
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(signer_key_pair, transaction))
	print('Mosaic alias transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')

	# Announce and confirm transaction
	announce_transaction(json_payload, 'mosaic alias transaction')
	wait_for_confirmation(transaction_hash, 'mosaic alias transaction')
	# [<step-5]
	# Retrieve the namespace to verify the alias [>step-6]
	namespace_path = f'/namespaces/{namespace_id:x}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(f'{NODE_URL}{namespace_path}') as response:
		response_json = json.loads(response.read().decode())
		namespace_info = response_json['namespace']
		print('Alias information:')
		alias_type = namespace_info['alias']['type']
		print(f'  Alias type: {alias_type}')
		if alias_type == 1:  # MOSAIC type
			aliased_mosaic_id = namespace_info['alias']['mosaicId']
			print(f'  Linked mosaic ID: {aliased_mosaic_id}')
	# [<step-6]
	# Send a transfer using the alias instead of a raw mosaic ID [>step-7]
	print(f'Using alias in transfer: {namespace_name}')

	# Convert namespace to mosaic alias ID
	mosaic_alias_id = generate_mosaic_alias_id(namespace_name)
	print(f'Mosaic ID (alias):'
		f' {mosaic_alias_id} ({hex(mosaic_alias_id)})')

	test_transaction = facade.transaction_factory.create({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'recipient_address':
			facade.network.public_key_to_address(
				signer_key_pair.public_key),
		'mosaics': [{
			'mosaic_id': mosaic_alias_id,
			'amount': 1
		}]
	})
	test_transaction.fee = Amount(fee_multiplier * test_transaction.size)
	test_json_payload = facade.transaction_factory.attach_signature(
		test_transaction,
		facade.sign_transaction(signer_key_pair, test_transaction))
	print('Test transaction:')
	print(json.dumps(test_transaction.to_json(), indent=2))
	test_transaction_hash = facade.hash_transaction(test_transaction)
	print(f'Transaction hash: {test_transaction_hash}')
	announce_transaction(test_json_payload, 'test transaction')
	wait_for_confirmation(test_transaction_hash, 'test transaction')
	# [<step-7]
except Exception as e:
	print(e)
