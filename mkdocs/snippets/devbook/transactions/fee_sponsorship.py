import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Hash256
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id

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


APP_PRIVATE_KEY = os.getenv('APP_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
app_key_pair = SymbolFacade.KeyPair(
	PrivateKey(APP_PRIVATE_KEY))
print(f'App public key: {app_key_pair.public_key}')
USER_PRIVATE_KEY = os.getenv('USER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000099')
user_key_pair = SymbolFacade.KeyPair(
	PrivateKey(USER_PRIVATE_KEY))
print(f'User public key: {user_key_pair.public_key}')

facade = SymbolFacade('testnet')


# OPTION 1 [>step-1]
def build_prefunded_message_transaction(recipient_address, message):
	# Build the embedded message transaction [>step-2]
	message_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': recipient_address,
				'message': message
			},
			user_key_pair.public_key))
	# [<step-2]
	# Build the embedded prefund transaction [>step-3]
	prefund_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': facade.network.public_key_to_address(
					user_key_pair.public_key),
				'mosaics': [{
					'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
					'amount': 0  # To be filled once value is known
				}]
			},
			app_key_pair.public_key))
	# [<step-3]
	# Build the wrapper complete aggregate transaction [>step-4]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				[message_transaction, prefund_transaction]),
			'transactions': [message_transaction, prefund_transaction]
		},
		user_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60,
		1)
	# Update the prefund amount to match the total fee
	prefund_transaction.mosaics[0].amount = transaction.fee
	# Update the embedded transaction hashes
	transaction.transactions_hash = Hash256(
		facade.hash_embedded_transactions(
			[message_transaction, prefund_transaction]).bytes)
	# [<step-4]
	# Sign the aggregate transaction using the user's signature [>step-5]
	facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(user_key_pair, transaction))
	# Attach the app's cosignature
	transaction.cosignatures.append(
		facade.cosign_transaction(app_key_pair, transaction))
	# Obtain the payload
	json_payload = facade.transaction_factory.to_json(transaction)
	# [<step-5]
	return (transaction, json_payload)
# [<step-1]


# OPTION 2 [>step-6]
def build_sponsored_message_transaction(recipient_address, message):
	# Build the embedded message transaction [>step-7]
	message_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': recipient_address,
				'message': message
			},
			user_key_pair.public_key))
	# [<step-7]
	# Build the embedded filler transaction [>step-8]
	filler_transaction = (
		facade.create_embedded_transaction_from_descriptor(
			{
				'type': 'transfer_transaction_v1',
				'recipient_address': facade.network.public_key_to_address(
					app_key_pair.public_key)
			},
			app_key_pair.public_key))
	# [<step-8]
	# Build the wrapper complete aggregate transaction [>step-9]
	transaction = facade.create_transaction_from_descriptor(
		{
			'type': 'aggregate_complete_transaction_v3',
			'transactions_hash': facade.hash_embedded_transactions(
				[message_transaction, filler_transaction]),
			'transactions': [message_transaction, filler_transaction]
		},
		app_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60,
		1)
	# [<step-9]
	# Sign the aggregate transaction using the app's signature [>step-10]
	facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(app_key_pair, transaction))
	# Attach the users's cosignature
	transaction.cosignatures.append(
		facade.cosign_transaction(user_key_pair, transaction))
	# Obtain the payload
	json_payload = facade.transaction_factory.to_json(transaction)
	# [<step-10]

	return (transaction, json_payload)  # [<step-6]


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

	# Choose one
	(agg_transaction,
		agg_json_payload) = build_prefunded_message_transaction(
			'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I', 'Hello world!')
	# (agg_transaction,
	# 	agg_json_payload) = build_sponsored_message_transaction(
	# 		'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I', 'Hello world!')

	print('Built transaction:')
	print(json.dumps(agg_transaction.to_json(), indent=2))

	# Announce the transaction
	announce_transaction(agg_json_payload, 'transaction')

	# Wait for confirmation
	transaction_hash = facade.hash_transaction(agg_transaction)
	print(f'Transaction hash: {transaction_hash}')
	wait_for_confirmation(transaction_hash, 'transaction')

except urllib.error.URLError as e:
	print(e.reason)
