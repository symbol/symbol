import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade

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


# Helper function to fetch account mosaic balances
def get_account_mosaics(address):
	account_path = f'/accounts/{address}'
	print(f'Fetching account information from {account_path}')
	with urllib.request.urlopen(f'{NODE_URL}{account_path}') as resp:
		resp_json = json.loads(resp.read().decode())
		return resp_json['account']['mosaics']


SIGNER_PRIVATE_KEY = os.getenv('SIGNER_PRIVATE_KEY',  # [>step-1]
	'0000000000000000000000000000000000000000000000000000000000000000')
signer_key_pair = SymbolFacade.KeyPair(PrivateKey(SIGNER_PRIVATE_KEY))

facade = SymbolFacade('testnet')
signer_address = facade.network.public_key_to_address(
	signer_key_pair.public_key)
print(f'Signer address: {signer_address}')

SOURCE_ADDRESS = os.getenv('SOURCE_ADDRESS',
	'TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA')
print(f'Source address: {SOURCE_ADDRESS}')

MOSAIC_ID_HEX = os.getenv('MOSAIC_ID', '7AED3D514C986941')
mosaic_id = int(MOSAIC_ID_HEX, 16)
print(f'Mosaic ID: {mosaic_id} (0x{mosaic_id:016X})')
# [<step-1]
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
	# --- CHECKING INITIAL BALANCE ---
	print('\n--- Checking initial balance ---')
	mosaics = get_account_mosaics(SOURCE_ADDRESS)  # [>step-3]
	for mosaic in mosaics:
		if mosaic['id'] == MOSAIC_ID_HEX.upper():
			print(f'  Mosaic ID: {mosaic["id"]},'
				f' Amount: {mosaic["amount"]}')
	# [<step-3]
	# --- REVOKING MOSAIC ---
	print('\n--- Revoking mosaic ---')
	# [>step-4]
	revoke_tx = facade.create_transaction_from_descriptor(
		{
			'type': 'mosaic_supply_revocation_transaction_v1',
			'source_address': SOURCE_ADDRESS,
			'mosaic': {
				'mosaic_id': mosaic_id,
				'amount': 7_00
			}
		},
		signer_key_pair.public_key,
		fee_multiplier,
		2 * 60 * 60)
	# [<step-4]
	# Sign and generate final payload [>step-5]
	signature = facade.sign_transaction(
		signer_key_pair, revoke_tx)
	json_payload = facade.transaction_factory.attach_signature(
		revoke_tx, signature)
	print('Built mosaic revocation transaction:')
	print(json.dumps(revoke_tx.to_json(), indent=2))

	revoke_hash = facade.hash_transaction(revoke_tx)
	print(f'Transaction hash: {revoke_hash}')

	# Announce transaction
	announce_transaction(json_payload, 'mosaic revocation')
	# [<step-5]
	# Wait for confirmation [>step-6]
	wait_for_confirmation(revoke_hash, 'mosaic revocation')
	# [<step-6]
	# --- VERIFYING REVOCATION ---
	print('\n--- Verifying revocation ---')
	mosaics = get_account_mosaics(SOURCE_ADDRESS)  # [>step-7]
	for mosaic in mosaics:
		if mosaic['id'] == MOSAIC_ID_HEX.upper():
			print(f'  Mosaic ID: {mosaic["id"]},'
				f' Amount: {mosaic["amount"]}')
	# [<step-7]
except Exception as e:
	print(e)
