import json
import os
import time
import urllib.request
import hashlib

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.environ.get(
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

# Returns a filtered list of restrictions currently applied to the mosaic
# matching the given restriction key
def get_mosaic_restrictions(query, key):
	restrictions_path = f'/restrictions/mosaic?{query}'
	print(f'  Getting restrictions from {restrictions_path}')
	res = []
	try:
		url = f'{NODE_URL}{restrictions_path}'
		with urllib.request.urlopen(url) as response:
			status = json.loads(response.read().decode())
			data = status['data']
			if len(data) > 0:
				# Look at the first returned restriction
				rlist = data[0]['mosaicRestrictionEntry']['restrictions']
				# Filter by key
				res = [r for r in rlist if int(r['key']) == key]
	except urllib.error.HTTPError:
		# The mosaic has no restrictions applied to this key
		pass
	print(f'  Response: {res}')
	return res

def get_mosaic_global_restrictions(mosaic_id, key):
	return get_mosaic_restrictions(
		f'mosaicId={mosaic_id:X}&entryType=1', key)

def get_mosaic_address_restrictions(mosaic_id, address, key):
	return get_mosaic_restrictions(
		f'mosaicId={mosaic_id:X}&entryType=0&targetAddress={address}',
		key)

# Returns a transaction enabling a mosaic's global restriction
def global_restriction_enable_transaction():
	transaction = facade.transaction_factory.create_embedded({
		'type': 'mosaic_global_restriction_transaction_v1',
		'signer_public_key': owner_key_pair.public_key,
		'mosaic_id': mosaic_id,
		'reference_mosaic_id': 0,
		'restriction_key': restriction_key,
		'previous_restriction_type': 0,
		'previous_restriction_value': 0,
		'new_restriction_type': 'ge',
		'new_restriction_value': 1
	})
	print(json.dumps(transaction.to_json(), indent=2))

	return transaction

# Returns a transaction setting an address restriction's value
def address_restriction_set_value(prev_value, new_value, address):
	transaction = facade.transaction_factory.create_embedded({
		'type': 'mosaic_address_restriction_transaction_v1',
		'signer_public_key': owner_key_pair.public_key,
		'mosaic_id': mosaic_id,
		'restriction_key': restriction_key,
		'previous_restriction_value': prev_value,
		'new_restriction_value': new_value,
		'target_address': address
	})
	print(json.dumps(transaction.to_json(), indent=2))

	return transaction

facade = SymbolFacade('testnet')

OWNER_PRIVATE_KEY = os.getenv('OWNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
owner_key_pair = SymbolFacade.KeyPair(PrivateKey(OWNER_PRIVATE_KEY))
owner_address = facade.network.public_key_to_address(
	owner_key_pair.public_key)
print(f'Owner address: {owner_address}')

target_address = os.getenv('TARGET_ADDRESS',
	'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA')
print(f'Target address: {target_address}')

mosaic_id = int(os.getenv('MOSAIC_ID', '6A383620F5C7A5B2'), 16)
print(f'Mosaic ID: 0x{mosaic_id:08X}')
restriction_name = os.getenv('RESTRICTION_NAME', 'security_level')
hasher = hashlib.sha3_256()
hasher.update(restriction_name.encode('utf8'))
restriction_key = int.from_bytes(hasher.digest()[:4])
print(f'Restriction name: "{restriction_name}"'
	' (key: 0x{restriction_key:08X})')

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

	# Enable global restriction if required
	transactions = []
	print("Checking if the global restriction is enabled:")
	global_restrictions = get_mosaic_global_restrictions(
		mosaic_id, restriction_key)
	if len(global_restrictions) == 0:
		# Enable the global restriction
		print('+ Enabling global restriction')
		transactions.append(global_restriction_enable_transaction())

		# Enable the address restriction
		print('+ Authorizing owner account')
		transactions.append(address_restriction_set_value(
			0xFFFFFFFF_FFFFFFFF, 1, owner_address))

	# Toggle target address restriction
	print("Checking if target account is authorized:")
	address_restrictions = get_mosaic_address_restrictions(
		mosaic_id, target_address, restriction_key)
	prev_value = 0xFFFFFFFF_FFFFFFFF
	if len(address_restrictions) > 0:
		prev_value = int(address_restrictions[0]['value'])
	if prev_value != 1:
		# Enable the address restriction
		print('+ Authorizing target account')
		transactions.append(address_restriction_set_value(
			prev_value, 1, target_address))
	else:
		# Disable the address restriction
		print('+ Deauthorizing target account')
		transactions.append(address_restriction_set_value(
			prev_value, 0, target_address))

	# Build an aggregate transaction
	print('Bundling', len(transactions),'transaction(s) in an aggregate')
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		'signer_public_key': owner_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			transactions),
		'transactions': transactions
	})
	transaction.fee = Amount(fee_mult * transaction.size)

	# Sign, announce and wait for confirmation
	payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(owner_key_pair, transaction))
	transaction_hash = facade.hash_transaction(transaction)
	announce_transaction(payload, 'aggregate')
	wait_for_confirmation(transaction_hash, 'aggregate')

	# Try to transfer the mosaic to the target address
	transaction = facade.transaction_factory.create({
		'type': 'transfer_transaction_v1',
		'signer_public_key': owner_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'recipient_address': target_address,
		'mosaics': [{
			'mosaic_id': mosaic_id,
			'amount': 1
		}]
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(owner_key_pair, transaction))
	transaction_hash = facade.hash_transaction(transaction)
	print('\nAttempting transfer to the target account')
	announce_transaction(payload, 'test transfer')
	wait_for_confirmation(transaction_hash, 'test transfer')

except Exception as e:
	print(e)
