import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain import sc


# OPTION 1

def build_prefunded_message_transaction(recipient_address, message):
	# Build the embedded message transaction
	message_transaction = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		# Account sending the message
		'signer_public_key': user_key_pair.public_key,
		'recipient_address': recipient_address,
		'message': message
	})

	# Build the embedded prefund transaction
	prefund_transaction = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		# Account funding the transaction fee
		'signer_public_key': app_key_pair.public_key,
		# Account receiving the funds
		'recipient_address':
			facade.network.public_key_to_address(
				user_key_pair.public_key),
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 0 # To be filled once value is known
		}]
	})

	# Build the wrapper complete aggregate transaction
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		# This is the account that will pay for the transaction
		'signer_public_key': user_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions': [message_transaction, prefund_transaction]
	})
	# Calculate total fee, reserving space for a cosignature
	transaction.fee = sc.Amount(fee_mult * (transaction.size + 104))
	# Update the prefund amount to match the total fee
	prefund_transaction.mosaics[0].amount = transaction.fee
	# Update the embedded transaction hashes
	transaction.transactions_hash = sc.Hash256(
		facade.hash_embedded_transactions(
			[message_transaction, prefund_transaction]).bytes)

	# Sign the aggregate transaction using the user's signature
	facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(user_key_pair, transaction))
	# Attach the app's cosignature
	transaction.cosignatures.append(
		facade.cosign_transaction(app_key_pair, transaction))
	# Obtain the payload
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(user_key_pair, transaction))

	return (transaction, json_payload)

# OPTION 2

def build_sponsored_message_transaction(recipient_address, message):
	# Build the embedded message transaction
	message_transaction = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		# Account sending the message
		'signer_public_key': user_key_pair.public_key,
		'recipient_address': recipient_address,
		'message': message
	})

	# Build the embedded filler transaction
	filler_transaction = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		# The application account is both the sender and the recipient
		# and there is no `mosaics` field
		'signer_public_key': app_key_pair.public_key,
		'recipient_address':
			facade.network.public_key_to_address(
				app_key_pair.public_key)
	})

	# Build the wrapper complete aggregate transaction
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		# This is the account that will pay for the transaction
		'signer_public_key': app_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			[message_transaction, filler_transaction]),
		'transactions': [message_transaction, filler_transaction]
	})
	# Calculate total fee, reserving space for a cosignature
	transaction.fee = sc.Amount(fee_mult * (transaction.size + 104))

	# Sign the aggregate transaction using the app's signature
	facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(app_key_pair, transaction))
	# Attach the users's cosignature
	transaction.cosignatures.append(
		facade.cosign_transaction(user_key_pair, transaction))
	# Obtain the payload
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(app_key_pair, transaction))

	return (transaction, json_payload)

NODE_URL = 'https://reference.symboltest.net:3001'
print(f'Using node {NODE_URL}')


APP_PRIVATE_KEY = os.getenv(
	'APP_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
app_key_pair = SymbolFacade.KeyPair(
	PrivateKey(APP_PRIVATE_KEY))
print(f'App public key: {app_key_pair.public_key}')
USER_PRIVATE_KEY = os.getenv(
	'USER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000099')
user_key_pair = SymbolFacade.KeyPair(
	PrivateKey(USER_PRIVATE_KEY))
print(f'User public key: {user_key_pair.public_key}')

facade = SymbolFacade('testnet')

try:
	# Fetch current network time
	time_path = '/node/time'
	print(f'Fetching current network time from {time_path}')
	with urllib.request.urlopen(f'{NODE_URL}{time_path}') as response:
		response_json = json.loads(response.read().decode())
		timestamp = NetworkTimestamp(int(
			response_json['communicationTimestamps']['receiveTimestamp']))
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

	# Choose one
	(transaction, json_payload) = build_prefunded_message_transaction(
		'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I', 'Hello world!')
	#(transaction, json_payload) = build_sponsored_message_transaction(
	#	'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I', 'Hello world!')

	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))

	# Announce the transaction
	announce_path = '/transactions'
	print(f'Announcing transaction to {announce_path}')
	announce_request = urllib.request.Request(
		f'{NODE_URL}{announce_path}',
		data=json_payload.encode(),
		headers={ 'Content-Type': 'application/json' },
		method='PUT'
	)
	with urllib.request.urlopen(announce_request) as response:
		print(f'  Response: {response.read().decode()}')

	# Wait for confirmation
	status_path = (
		f'/transactionStatus/{facade.hash_transaction(transaction)}')
	print(f'Waiting for confirmation from {status_path}')
	for attempt in range(60):
		time.sleep(1)
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status['group']}')
				if status['group'] == 'confirmed':
					print(f'Transaction confirmed in {attempt} seconds')
					break
				if status['group'] == 'failed':
					print(f'Transaction failed: {status['code']}')
					break
		except urllib.error.HTTPError as e:
			print(f'  Transaction status: unknown | Cause: ({e.msg})')
	else:
		print('Confirmation took too long.')

except urllib.error.URLError as e:
	print(e.reason)