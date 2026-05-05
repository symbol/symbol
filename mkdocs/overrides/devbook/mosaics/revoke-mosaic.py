import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.sc import Amount

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')


# Helper function to fetch account mosaic balances
def get_account_mosaics(address):
	account_path = f'/accounts/{address}'
	print(f'Fetching account information from {account_path}')
	with urllib.request.urlopen(f'{NODE_URL}{account_path}') as response:
		response_json = json.loads(response.read().decode())
		return response_json['account']['mosaics']

# [>step-1]
SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

SOURCE_ADDRESS = os.getenv('SOURCE_ADDRESS',
	'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA')
print(f'Source address: {SOURCE_ADDRESS}')

MOSAIC_ID_HEX = os.getenv('MOSAIC_ID', '7aed3d514c986941')
mosaic_id = int(MOSAIC_ID_HEX, 16)
print(f'Mosaic ID: {mosaic_id} (0x{MOSAIC_ID_HEX})')
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
	# --- CHECKING INITIAL BALANCE ---
	print('\n--- Checking initial balance ---')
	mosaics = get_account_mosaics(SOURCE_ADDRESS) # [>step-3]
	for mosaic in mosaics:
		if mosaic['id'] == MOSAIC_ID_HEX.upper():
			print(f'  Mosaic ID: {mosaic["id"]},'
				f' Amount: {mosaic["amount"]}')
	# [<step-3]
	# --- REVOKING MOSAIC ---
	print('\n--- Revoking mosaic ---')
	# [>step-4]
	revoke_tx = facade.transaction_factory.create({
		'type': 'mosaic_supply_revocation_transaction_v1',
		'signer_public_key': signer_key_pair.public_key,
		'deadline': timestamp.add_hours(2).timestamp,
		'source_address': SOURCE_ADDRESS,
		'mosaic': {
			'mosaic_id': mosaic_id,
			'amount': 7_00
		}
	})
	revoke_tx.fee = Amount(fee_mult * revoke_tx.size)
	# [<step-4]
	# Sign and generate final payload [>step-5]
	signature = facade.sign_transaction(
		signer_key_pair, revoke_tx)
	json_payload = facade.transaction_factory.attach_signature(
		revoke_tx, signature)
	print('Built mosaic revocation transaction:')
	print(json.dumps(revoke_tx.to_json(), indent=2))

	# Announce transaction
	revoke_hash = facade.hash_transaction(revoke_tx)
	print(f'Transaction hash: {revoke_hash}')

	print('Announcing mosaic revocation to /transactions')
	request = urllib.request.Request(
		f'{NODE_URL}/transactions',
		data=json_payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')
	# [<step-5]
	# Wait for confirmation [>step-6]
	print('Waiting for mosaic revocation confirmation...')
	for attempt in range(60):
		time.sleep(1)
		try:
			status_url = f'{NODE_URL}/transactionStatus/{revoke_hash}'
			with urllib.request.urlopen(status_url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')
			if status['group'] == 'confirmed':
				print('Mosaic revocation confirmed in',
					attempt, 'seconds')
				break
			if status['group'] == 'failed':
				raise Exception(
					f'Mosaic revocation failed: {status["code"]}')
		except urllib.error.HTTPError:
			print('  Transaction status: unknown')
	# [<step-6]
	# --- VERIFYING REVOCATION ---
	print('\n--- Verifying revocation ---')
	mosaics = get_account_mosaics(SOURCE_ADDRESS) # [>step-7]
	for mosaic in mosaics:
		if mosaic['id'] == MOSAIC_ID_HEX.upper():
			print(f'  Mosaic ID: {mosaic["id"]},'
				f' Amount: {mosaic["amount"]}')
	# [<step-7]
except Exception as e:
	print(e)
