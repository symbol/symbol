import datetime
import json
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.sc import Amount

NODE_URL = 'https://001-sai-dual.symboltest.net:3001'
print(f'Using node {NODE_URL}')

SIGNER_PRIVATE_KEY = (
	'EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

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

	# Build the transaction
	transaction = facade.transaction_factory.create({
		'type': 'transfer_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'recipient_address':
			facade.network.public_key_to_address(
				signer_key_pair.public_key),
		'mosaics': [{
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 1_000_000 # 1 XYM
		}]
	})
	transaction.fee = Amount(fee_mult * transaction.size)

	# Sign transaction and generate final payload
	signature = facade.sign_transaction(signer_key_pair, transaction)
	json_payload = facade.transaction_factory.attach_signature(
		transaction, signature)
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