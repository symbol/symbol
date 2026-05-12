import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import Address, SymbolFacade
from symbolchain.sc import AccountRestrictionFlags, Amount
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

facade = SymbolFacade('testnet')

SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',  # [>step-1]
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

auth_address = 'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA'
print(f'Authorized address: {auth_address}')  # [<step-1]


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


# Returns the list of restrictions currently applied to the account
def get_account_restrictions(address):  # [>step-3]
	restrictions_path = f'/restrictions/account/{address}'
	print(f'Getting restrictions from {restrictions_path}')
	try:
		url = f'{NODE_URL}{restrictions_path}'
		with urllib.request.urlopen(url) as restr_response:
			status = json.loads(restr_response.read().decode())
			found_restr = status['accountRestrictions']['restrictions']
			print(f'  Response: {found_restr}')
			return found_restr
	except urllib.error.HTTPError:
		# The address has never been used
		print('  Response: No restrictions found')
	return []  # [<step-3]


# Returns a transaction that restricts an account
def restriction_enable_transaction():  # [>step-5]
	enable_transaction = facade.transaction_factory.create({
		'type': 'account_address_restriction_transaction_v1',
		# This is the account that will be restricted
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		# Allow only OUTGOING transactions to the authorized ADDRESS
		'restriction_flags':
			AccountRestrictionFlags.ADDRESS |
			AccountRestrictionFlags.OUTGOING,
		# This is the only authorized outgoing address
		'restriction_additions': [auth_address]
	})
	enable_transaction.fee = Amount(fee_mult * enable_transaction.size)
	print('Enabling the restriction with transaction:')
	print(json.dumps(enable_transaction.to_json(), indent=2))

	return enable_transaction  # [<step-5]


# Returns a transaction that removes a restriction from an account
def restriction_disable_transaction(restriction):  # [>step-6]
	disable_transaction = facade.transaction_factory.create({
		'type': 'account_address_restriction_transaction_v1',
		# This is the account whose restriction will be lifted
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		# Reverse flags
		'restriction_flags': restriction['restrictionFlags'],
		# Remove all addresses currently restricted
		'restriction_deletions': [
			Address.from_decoded_address_hex_string(addr)
			for addr in restriction['values']
		]
	})
	disable_transaction.fee = Amount(fee_mult * disable_transaction.size)
	print('Disabling the restriction with transaction:')
	print(json.dumps(disable_transaction.to_json(), indent=2))

	return disable_transaction  # [<step-6]


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
	# Get current state of the restriction and decide which
	# operation to perform
	restrictions = get_account_restrictions(signer_address)  # [>step-4]
	if len(restrictions) == 0:
		# Enable the restriction
		print('\n--- Enabling restriction ---')
		agg_transaction = restriction_enable_transaction()
	else:
		# Disable the restriction
		print('\n--- Disabling restriction ---')
		agg_transaction = restriction_disable_transaction(restrictions[0])
	# [<step-4]
	# Sign, announce and wait for confirmation [>step-7]
	json_payload = facade.transaction_factory.attach_signature(
		agg_transaction,
		facade.sign_transaction(signer_key_pair, agg_transaction))
	transaction_hash = facade.hash_transaction(agg_transaction)
	announce_transaction(json_payload, 'restriction transaction')
	wait_for_confirmation(transaction_hash, 'restriction transaction')
	# [<step-7]
	# Try a dummy transfer to a random address with no mosaics [>step-8]
	transaction = facade.transaction_factory.create({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'recipient_address': 'TBBHGE77IHHOIYA46B3XSORRNR2L5MLW54YO75Y'
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(signer_key_pair, transaction))
	transaction_hash = facade.hash_transaction(transaction)
	print('\n--- Attempting transfer to unauthorized address ---')
	announce_transaction(json_payload, 'test transfer')
	wait_for_confirmation(transaction_hash, 'test transfer')
	# [<step-8]
except Exception as e:
	print(e)
