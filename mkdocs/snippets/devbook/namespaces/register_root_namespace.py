import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_namespace_id
from symbolchain.symbol.Network import Address, NetworkTimestamp

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')
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
		median_multiplier = response_json['medianFeeMultiplier']
		minimum_multiplier = response_json['minFeeMultiplier']
		fee_multiplier = max(median_multiplier, minimum_multiplier)
		print(f'  Fee multiplier: {fee_multiplier}')
	# [<step-2]
	# Build the transaction [>step-3]
	namespace_name = f'ns_{int(time.time())}'
	print(f'Creating root namespace: {namespace_name}')

	transaction = facade.transaction_factory.create({
		'type': 'namespace_registration_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'registration_type': 'root',
		'duration': 86400,  # approximately 30 days
		'name': namespace_name
	})
	transaction.fee = Amount(fee_multiplier * transaction.size)
	# [<step-3]
	# Sign transaction and generate final payload [>step-4]
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')

	# Announce transaction
	print('Announcing namespace registration to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')
	# [<step-4]
	# Wait for confirmation [>step-5]
	print('Waiting for namespace registration confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			status_url = (
				f'{NODE_URL}/transactionStatus/{transaction_hash}')
			with urllib.request.urlopen(status_url) as response:
				status = json.loads(response.read().decode())
				print(f"  Transaction status: {status['group']}")
			if status['group'] == 'confirmed':
				print('Namespace registration confirmed in',
					attempt, 'seconds')
				break
			if status['group'] == 'failed':
				raise RuntimeError('Namespace registration failed:',
					status['code'])
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	# [<step-5]
	# Retrieve the namespace [>step-6]
	namespace_id = generate_namespace_id(namespace_name)
	print(f'Namespace ID: {namespace_id} ({hex(namespace_id)})')

	namespace_path = f'/namespaces/{namespace_id:x}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(f'{NODE_URL}{namespace_path}') as response:
		response_json = json.loads(response.read().decode())
		namespace_info = response_json['namespace']
		print('Namespace information:')
		reg_type = namespace_info['registrationType']
		print(f'  Registration type: {reg_type}')
		owner_address = Address.from_decoded_address_hex_string(
			namespace_info['ownerAddress'])
		print(f'  Owner address: {owner_address}')
		print(f"  Start height: {namespace_info['startHeight']}")  # [<step-6]
		print(f"  End height: {namespace_info['endHeight']}")

except Exception as e:
	print(e)
