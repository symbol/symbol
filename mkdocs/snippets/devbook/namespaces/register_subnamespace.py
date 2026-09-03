import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_namespace_id
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


SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

try:
	# Fetch recommended fees
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(f'{NODE_URL}{fee_path}') as response:
		response_json = json.loads(response.read().decode())
		median_multiplier = response_json['medianFeeMultiplier']
		minimum_multiplier = response_json['minFeeMultiplier']
		fee_multiplier = max(median_multiplier, minimum_multiplier)
		print(f'  Fee multiplier: {fee_multiplier}')

	# Build the subnamespace name [>step-1]
	root_namespace_name = os.getenv('ROOT_NAMESPACE', 'ns_root')
	subnamespace_name = os.getenv(
		'SUBNAMESPACE', f'sub_{int(time.time())}')
	full_namespace_name = (
		f'{root_namespace_name}.{subnamespace_name}')
	print(f'Creating subnamespace: {full_namespace_name}')

	# Generate the parent namespace ID from the root namespace name
	parent_id = generate_namespace_id(root_namespace_name)
	print(f'  Parent namespace ID: 0x{parent_id:016X}')
	# [<step-1]
	# Build the transaction [>step-2]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'namespace_registration_transaction_v1',
			'registration_type': 'child',
			'parent_id': parent_id,
			'name': subnamespace_name
		},
		signer_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)
	# [<step-2]
	# Sign transaction and generate final payload
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	transaction_hash = facade.hash_transaction(transaction)
	print(f'Transaction hash: {transaction_hash}')

	# Announce transaction
	announce_transaction(json_payload, 'namespace registration')

	# Wait for confirmation
	wait_for_confirmation(transaction_hash, 'namespace registration')

	# Retrieve the namespace [>step-3]
	namespace_id = generate_namespace_id(
		subnamespace_name, parent_id)
	print(f'Child namespace ID: {namespace_id} (0x{namespace_id:016X})')

	namespace_path = f'/namespaces/{namespace_id:016X}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(
		f'{NODE_URL}{namespace_path}'
	) as response:
		response_json = json.loads(response.read().decode())
		namespace_info = response_json['namespace']
		print('Namespace information:')
		reg_type = namespace_info['registrationType']
		print(f'  Registration type: {reg_type}')
		owner_address = Address.from_decoded_address_hex_string(
			namespace_info['ownerAddress'])
		print(f'  Owner address: {owner_address}')
		print(f"  Parent ID: {namespace_info['parentId']}")
		print(f"  Depth: {namespace_info['depth']}")
		print(f"  Level 0: {namespace_info['level0']}")
		if int(namespace_info['depth']) >= 1:
			print(f"  Level 1: {namespace_info['level1']}")
		if int(namespace_info['depth']) >= 2:
			if 'level2' in namespace_info:
				print(f"  Level 2: {namespace_info['level2']}")
		print(f"  Start height: {namespace_info['startHeight']}")
		print(f"  End height: {namespace_info['endHeight']}")  # [<step-3]

except Exception as e:
	print(e)
