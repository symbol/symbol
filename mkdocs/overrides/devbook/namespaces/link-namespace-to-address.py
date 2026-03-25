import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import (
	generate_namespace_path, generate_mosaic_alias_id)
from symbolchain.sc import Amount
from symbolchain.symbol.Network import Address

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

namespace_name = os.getenv('NAMESPACE_NAME', 'my_namespace')
print(f'Namespace name: {namespace_name}')

namespace_id = generate_namespace_path(namespace_name)[-1]
print(f'Namespace ID: {namespace_id} ({hex(namespace_id)})')

# Target address to link the namespace to
target_address = Address(
	os.getenv('TARGET_ADDRESS',
	'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI'))
print(f'Target address: {target_address}')

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

	# Build the alias transaction
	transaction = facade.transaction_factory.create({
		'type': 'address_alias_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'namespace_id': namespace_id,
		'address': target_address,
		'alias_action': 'link'
	})
	transaction.fee = Amount(fee_mult * transaction.size)

	# Sign transaction and generate final payload
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')

	# Announce transaction
	print('Announcing address alias transaction to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')

	# Wait for confirmation
	print('Waiting for transaction confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			status_url = (
				f'{NODE_URL}/transactionStatus/{transaction_hash}')
			with urllib.request.urlopen(status_url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
			if status['group'] == 'confirmed':
				print('Address alias transaction confirmed in',
					attempt, 'seconds')
				break
			if status['group'] == 'failed':
				raise Exception('Address alias transaction failed:',
					status['code'])
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')

	# Retrieve the namespace to verify the alias
	namespace_path = f'/namespaces/{namespace_id:x}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{namespace_path}') as response:
		response_json = json.loads(response.read().decode())
		namespace_info = response_json['namespace']
		print('Alias information:')
		alias_type = namespace_info['alias']['type']
		print(f'  Alias type: {alias_type}')
		if alias_type == 2:  # ADDRESS type
			aliased_address = Address.from_decoded_address_hex_string(
				namespace_info['alias']['address'])
			print(f'  Linked address: {aliased_address}')

	# Send a transfer using the alias instead of a raw address
	print(f'Using alias in transfer: {namespace_name}')

	# Encode the namespace ID as a recipient address (24 bytes)
	# Byte 0: network byte (Network Identifier | 0x01 to indicate alias)
	# Bytes 1-8: namespace ID in little-endian
	# Bytes 9-23: zero padding
	recipient_id = generate_namespace_path(namespace_name)[-1]
	recipient_address = Address(
		bytes([facade.network.identifier | 0x01]) +
		recipient_id.to_bytes(8, 'little') + bytes(15))

	transfer_tx = facade.transaction_factory.create({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'recipient_address': recipient_address,
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 1_000_000  # 1 XYM
		}]
	})
	print('Transfer transaction:')
	print(f'  Recipient address (alias): {recipient_address}')

except Exception as e:
	print(e)
