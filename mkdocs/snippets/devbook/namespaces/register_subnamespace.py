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

SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

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

	# Build the transaction [>step-1]
	root_namespace_name = 'ns_root'
	child_namespace_name = f'sub_{int(time.time())}'
	full_namespace_name = (
		f'{root_namespace_name}.{child_namespace_name}')
	print(f'Creating child namespace: {full_namespace_name}')

	# Generate the parent namespace ID from the root namespace name
	parent_id = generate_namespace_id(root_namespace_name)
	print(f'  Parent namespace ID: {hex(parent_id)}')

	transaction = facade.transaction_factory.create({
		'type': 'namespace_registration_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'registration_type': 'child',
		'parent_id': parent_id,
		'name': child_namespace_name
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	# [<step-1]
	# Sign transaction and generate final payload
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

	# Wait for confirmation
	print('Waiting for namespace registration confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			status_url = (
				f'{NODE_URL}/transactionStatus/{transaction_hash}')
			with urllib.request.urlopen(status_url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
			if status['group'] == 'confirmed':
				print('Namespace registration confirmed in',
					attempt, 'seconds')
				break
			if status['group'] == 'failed':
				raise Exception('Namespace registration failed:',
					status['code'])
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')

	# Retrieve the namespace [>step-2]
	namespace_id = generate_namespace_id(
		child_namespace_name, parent_id)
	print(f'Child namespace ID: {namespace_id} ({hex(namespace_id)})')

	namespace_path = f'/namespaces/{namespace_id:x}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(f'{NODE_URL}{namespace_path}') as response:
		response_json = json.loads(response.read().decode())
		namespace_info = response_json['namespace']
		print('Namespace information:')
		reg_type = namespace_info["registrationType"]
		print(f'  Registration type: {reg_type}')
		owner_address = Address.from_decoded_address_hex_string(
			namespace_info["ownerAddress"])
		print(f'  Owner address: {owner_address}')
		print(f'  Parent ID: {namespace_info["parentId"]}')
		print(f'  Depth: {namespace_info["depth"]}')
		print(f'  Level 0: {namespace_info["level0"]}')
		if int(namespace_info['depth']) >= 1:
			print(f'  Level 1: {namespace_info["level1"]}')
		if int(namespace_info['depth']) >= 2:
			if 'level2' in namespace_info:
				print(f'  Level 2: {namespace_info["level2"]}')
		print(f'  Start height: {namespace_info["startHeight"]}')  # [<step-2]
		print(f'  End height: {namespace_info["endHeight"]}')

except Exception as e:
	print(e)
