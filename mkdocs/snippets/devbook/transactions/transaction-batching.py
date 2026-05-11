import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.sc import Amount
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import Address, NetworkTimestamp

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')
# [>step-1]
SIGNER_PRIVATE_KEY = os.getenv(
	'SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(
	PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer public key: {signer_key_pair.public_key}')
print(f'Signer address: {signer_address}')

RECIPIENT_1 = os.getenv(
	'RECIPIENT_1', 'TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI')
RECIPIENT_2 = os.getenv(
	'RECIPIENT_2', 'TCD4NC5VIE2EEB3BCV5JRLBNJXYDW5Q5JK547MI')
recipient1_hex = Address(RECIPIENT_1).bytes.hex().upper()
recipient2_hex = Address(RECIPIENT_2).bytes.hex().upper()
print(f'Recipient 1: {RECIPIENT_1} ({recipient1_hex})')
print(f'Recipient 2: {RECIPIENT_2} ({recipient2_hex})')
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
		median_mult = response_json['medianFeeMultiplier']
		minimum_mult = response_json['minFeeMultiplier']
		fee_mult = max(median_mult, minimum_mult)
		print(f'  Fee multiplier: {fee_mult}')
	# [<step-2]
	# Embedded tx 1: Send 5 XYM to Recipient 1 [>step-3]
	xym_mosaic_id = generate_mosaic_alias_id('symbol.xym')
	embedded_tx_1 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'recipient_address': RECIPIENT_1,
		'mosaics': [{
			'mosaic_id': xym_mosaic_id,
			'amount': 5_000_000  # 5 XYM
		}]
	})

	# Embedded tx 2: Send 3 XYM to Recipient 2
	embedded_tx_2 = facade.transaction_factory.create_embedded({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'recipient_address': RECIPIENT_2,
		'mosaics': [{
			'mosaic_id': xym_mosaic_id,
			'amount': 3_000_000  # 3 XYM
		}]
	})
	# [<step-3]
	# Build the aggregate transaction [>step-4]
	embedded_transactions = [embedded_tx_1, embedded_tx_2]
	transaction = facade.transaction_factory.create({
		'type': 'aggregate_complete_transaction_v3',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'transactions_hash':
			facade.hash_embedded_transactions(embedded_transactions),
		'transactions': embedded_transactions
	})
	transaction.fee = Amount(fee_mult * transaction.size)
	print('Built aggregate transaction:')
	print(json.dumps(transaction.to_json(), indent=2))
	# [<step-4]
	# Sign transaction and generate final payload [>step-5]
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = (facade.transaction_factory.attach_signature(
			transaction, signature))

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
	# [<step-5]
	# Wait for confirmation [>step-6]
	transaction_hash = facade.hash_transaction(transaction)
	status_path = f'/transactionStatus/{transaction_hash}'
	print(f'Waiting for confirmation from {status_path}')
	for attempt in range(60):
		time.sleep(1)
		try:
			with urllib.request.urlopen(
				f'{NODE_URL}{status_path}'
			) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
				if status['group'] == 'confirmed':
					print(f'Transaction confirmed in {attempt} seconds')
					break
				if status['group'] == 'failed':
					print(f'Transaction failed: {status["code"]}')
					break
		except urllib.error.HTTPError as e:
			print(f'  Transaction status: unknown | Cause: ({e.msg})')
	else:
		print('Confirmation took too long.')
	# [<step-6]
except Exception as e:
	print(e)
