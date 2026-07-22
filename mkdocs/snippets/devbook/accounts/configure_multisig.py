import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount, Cosignature
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

facade = SymbolFacade('testnet')
# [>step-1]
KEY_TEMPLATE = '0' * 63 + '{}'

# Setup the keys for the multisig account and its two cosignatories
MULTISIG_PRIVATE_KEY = os.getenv(
	'MULTISIG_PRIVATE_KEY', KEY_TEMPLATE.format(1))
multisig_key_pair = SymbolFacade.KeyPair(PrivateKey(MULTISIG_PRIVATE_KEY))
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
					raise RuntimeError(f'{label} failed: {status["code"]}')
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	raise TimeoutError(f'{label} not confirmed after 60 seconds')


# Returns the cosignatory addresses of the provided multisig account, [>step-3]
# or an empty list if the account is not multisig or has never been used
def get_multisig_cosignatories(address):
	multisig_path = f'/account/{address}/multisig'
	print(f'Getting cosignatories from {multisig_path}')
	try:
		url = f'{NODE_URL}{multisig_path}'
		with urllib.request.urlopen(url) as multisig_response:
			status = json.loads(multisig_response.read().decode())
			found_cosignatories = status['multisig']['cosignatoryAddresses']
			print(f'  Response: {found_cosignatories}')
			return found_cosignatories
	except urllib.error.HTTPError:
		# The address has never been used
		print('  Response: No cosignatories')
	return []  # [<step-3]


# Returns a transaction that turns a regular account into a multisig
def multisig_enable_transaction():
	# Create an embedded multisig account modification transaction [>step-5]
	# that adds two cosignatories
	embedded_transaction = facade.transaction_factory.create_embedded({
		'type': 'multisig_account_modification_transaction_v1',
		# This is the account that will be turned into a multisig
		'signer_public_key': multisig_key_pair.public_key,
		# Increment of the number of signatures required for approvals
		'min_approval_delta': 1,
		# Increment of the number of signatures required for removals
		'min_removal_delta': 1,
		'address_additions': cosignatory_addresses
	})
	# [<step-5]
	# Build the aggregate transaction [>step-6]
	embedded_transactions = [embedded_transaction]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		# This is the account that will pay for this transaction
		'signer_public_key': multisig_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	# Reserve space for two cosignatures
	# and calculate fee for the final transaction size
	cosignature_size = Cosignature().size
	transaction.fee = Amount(fee_multiplier *
		(transaction.size + cosignature_size * len(cosignatory_key_pairs)))
	print('Enabling the multisig with the aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-6]
	# Sign the aggregate transaction with the multisig's signature [>step-7]
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
	# Create two embedded multisig account modification transactions [>step-8]
	# because cosignatories must be removed one by one
	embedded_transaction_1 = facade.transaction_factory.create_embedded({
		'type': 'multisig_account_modification_transaction_v1',
		# This is the multisig account that will be modified
		'signer_public_key': multisig_key_pair.public_key,
		# Keep required signatures unchanged for this step
		'min_approval_delta': 0,
		'min_removal_delta': 0,
		'address_deletions': [cosignatory_addresses[1]]
	})
	embedded_transaction_2 = facade.transaction_factory.create_embedded({
		'type': 'multisig_account_modification_transaction_v1',
		# This is the multisig account that will be modified
		'signer_public_key': multisig_key_pair.public_key,
		# Decrease required signatures after final removal
		'min_approval_delta': -1,
		'min_removal_delta': -1,
		'address_deletions': [cosignatory_addresses[0]]
	})
	# [<step-8]
	# Build the aggregate transaction [>step-9]
	embedded_transactions = [embedded_transaction_1,
		embedded_transaction_2]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		# This is the account that will pay for all transactions
		'signer_public_key': cosignatory_key_pairs[0].public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			embedded_transactions),
		'transactions': embedded_transactions
	})
	# Calculate fee for the final transaction size
	# (No need to reserve space for cosignatures, as there are none)
	transaction.fee = Amount(fee_multiplier * transaction.size)
	print('Disabling the multisig with the aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	# Sign the aggregate transaction using the first cosigner's signature
	facade.transaction_factory.attach_signature(transaction,
		facade.sign_transaction(cosignatory_key_pairs[0], transaction))
	# [<step-9]
	return transaction


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
	# Get current state of the multisig account and decide which [>step-4]
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
	print(f'Built aggregate transaction with hash: {agg_transaction_hash}')
	announce_transaction(json_payload, 'aggregate transaction')
	wait_for_confirmation(agg_transaction_hash, 'aggregate transaction')
	# [<step-10]
except Exception as e:
	print(e)
