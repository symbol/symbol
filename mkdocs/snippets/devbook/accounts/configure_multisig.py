import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

facade = SymbolFacade('testnet')
# [>step-1]
KEY_TEMPLATE = '0' * 63 + '{}'

# Setup the keys for the multisig account and its two cosignatories
MULTISIG_PRIVATE_KEY = os.getenv(
	'MULTISIG_PRIVATE_KEY', KEY_TEMPLATE.format(1))
multisig_key_pair = SymbolFacade.KeyPair(
	PrivateKey(MULTISIG_PRIVATE_KEY))
multisig_address = facade.network.public_key_to_address(
	multisig_key_pair.public_key)
print(f'Multisig address: {multisig_address} '
	f'(public key {multisig_key_pair.public_key})')

cosignatory_key_pairs = []
cosignatory_addresses = []
for i in range(2):
	COSIGNATORY_PRIVATE_KEY = os.getenv(
		f'COSIGNATORY{i}_PRIVATE_KEY', KEY_TEMPLATE.format(i + 2))
	key_pair = SymbolFacade.KeyPair(PrivateKey(COSIGNATORY_PRIVATE_KEY))
	cosignatory_key_pairs.append(key_pair)
	addr = facade.network.public_key_to_address(key_pair.public_key)
	cosignatory_addresses.append(addr)
	print(f'Cosignatory {i} address: '
		f'{addr} (public key {key_pair.public_key})')  # [<step-1]


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


# [>step-3]
# Returns the cosignatory addresses of the provided multisig account,
# or an empty list if the account is not multisig or has never been used
def get_multisig_cosignatories(address):
	multisig_path = f'/account/{address}/multisig'
	print(f'Getting cosignatories from {multisig_path}')
	try:
		url = f'{NODE_URL}{multisig_path}'
		with urllib.request.urlopen(url) as multisig_response:
			status = json.loads(multisig_response.read().decode())
			found_cosignatories = (
				status['multisig']['cosignatoryAddresses'])
			print(f'  Response: {found_cosignatories}')
			return found_cosignatories
	except urllib.error.HTTPError:
		# The address has never been used
		print('  Response: No cosignatories')
	return []  # [<step-3]


# Returns a transaction that turns a regular account into a multisig
def multisig_enable_transaction():
	# [>step-5]
	# Create an embedded multisig account modification transaction
	# that adds two cosignatories
	embedded_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'multisig_account_modification_transaction_v1',
				# Increment of required signatures for removals
				'min_removal_delta': 1,
				# Increment of required signatures for approvals
				'min_approval_delta': 1,
				'address_additions': cosignatory_addresses
			},
			multisig_key_pair.public_key))
	# [<step-5]
	# Build the aggregate transaction [>step-6]
	embedded_transactions = [embedded_transaction]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				embedded_transactions),
			'transactions': embedded_transactions
		},
		multisig_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60,
		len(cosignatory_key_pairs))
	print('Enabling the multisig with the aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-6]
	# [>step-7]
	# Sign the aggregate transaction with the multisig's signature
	facade.transaction_factory.attach_signature(transaction,
		facade.sign_transaction(multisig_key_pair, transaction))

	# Append signatures from all cosignatories
	for cosignatory_key_pair in cosignatory_key_pairs:
		transaction.cosignatures.append(
			facade.cosign_transaction(cosignatory_key_pair, transaction)
		)
	# [<step-7]
	return transaction


# Returns a transaction that turns a multisig into a regular account
def multisig_disable_transaction():
	# [>step-8]
	# Create two embedded multisig account modification transactions
	# because cosignatories must be removed one by one
	embedded_transaction_1 = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'multisig_account_modification_transaction_v1',
				# Keep required signatures unchanged for this step
				'min_removal_delta': 0,
				'min_approval_delta': 0,
				'address_deletions': [cosignatory_addresses[1]]
			},
			multisig_key_pair.public_key))
	embedded_transaction_2 = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'multisig_account_modification_transaction_v1',
				# Decrease required signatures after final removal
				'min_removal_delta': -1,
				'min_approval_delta': -1,
				'address_deletions': [cosignatory_addresses[0]]
			},
			multisig_key_pair.public_key))
	# [<step-8]
	# Build the aggregate transaction [>step-9]
	embedded_transactions = [embedded_transaction_1,
		embedded_transaction_2]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				embedded_transactions),
			'transactions': embedded_transactions
		},
		cosignatory_key_pairs[0].public_key,
		fee_multiplier,
		2 * 60 * 60)
	print('Disabling the multisig with the aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	# Sign the aggregate transaction using the first cosigner's signature
	facade.transaction_factory.attach_signature(transaction,
		facade.sign_transaction(cosignatory_key_pairs[0], transaction))
	# [<step-9]
	return transaction


try:
	# Fetch recommended fees [>step-2]
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(f'{NODE_URL}{fee_path}') as response:
		response_json = json.loads(response.read().decode())
		median_multiplier = response_json['medianFeeMultiplier']
		minimum_multiplier = response_json['minFeeMultiplier']
		fee_multiplier = max(median_multiplier, minimum_multiplier)
		print(f'  Fee multiplier: {fee_multiplier}')
	# [<step-2]
	# [>step-4]
	# Get current state of the multisig account and decide which
	# operation to perform
	cosignatories = get_multisig_cosignatories(multisig_address)
	if len(cosignatories) == 0:
		# Enable the multisig
		agg_transaction = multisig_enable_transaction()
	else:
		# Disable the multisig
		agg_transaction = multisig_disable_transaction()
	json_payload = facade.transaction_factory.to_json(agg_transaction)
	# [<step-4]
	# Announce and wait for confirmation [>step-10]
	agg_transaction_hash = facade.hash_transaction(agg_transaction)
	print(
		f'Built aggregate transaction with hash: {agg_transaction_hash}')
	announce_transaction(json_payload, 'aggregate transaction')
	wait_for_confirmation(agg_transaction_hash, 'aggregate transaction')
	# [<step-10]
except Exception as e:
	print(e)
