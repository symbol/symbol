import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

# [>step-1]
MULTISIG_PRIVATE_KEY = os.getenv(
	'MULTISIG_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000001')
multisig_key_pair = SymbolFacade.KeyPair(
	PrivateKey(MULTISIG_PRIVATE_KEY))
print(f'Multisig public key: {multisig_key_pair.public_key}')
COSIGNATORY0_PRIVATE_KEY = os.getenv(
	'COSIGNATORY0_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000002')
cosignatory_key_pair = SymbolFacade.KeyPair(
	PrivateKey(COSIGNATORY0_PRIVATE_KEY))
print(f'Cosignatory public key: {cosignatory_key_pair.public_key}')
# [<step-1]
facade = SymbolFacade('testnet')

try:
	# Fetch current network time [>step-2]
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
	# [<step-2]
	# Build the embedded transfer transaction [>step-3]
	transfer_transaction = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': multisig_key_pair.public_key,
		'recipient_address':
			facade.network.public_key_to_address(
				multisig_key_pair.public_key),
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 1_000_000  # 1 XYM
		}]
	})
	# [<step-3]
	# Build the wrapper aggregate transaction [>step-4]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		# This is the account that will pay for the transaction
		'signer_public_key': cosignatory_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash': facade.hash_embedded_transactions(
			[transfer_transaction]),
		'transactions': [transfer_transaction]
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	# [<step-4]
	# Sign the aggregate transaction using the cosignatory's signature [>step-5]
	json_payload = facade.transaction_factory.attach_signature(
		transaction,
		facade.sign_transaction(cosignatory_key_pair, transaction))
	print('Built transaction:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-5]
	# Announce the transaction [>step-6]
	announce_path = '/transactions'
	print(f'Announcing transaction to {announce_path}')
	announce_request = urllib.request.Request(
		f'{NODE_URL}{announce_path}',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
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
	# [<step-6]
except urllib.error.URLError as e:
	print(e.reason)
