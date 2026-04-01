import hashlib
import json
import os
import time
import urllib.request

from symbolchain.CryptoTypes import Hash256, PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.sc import Amount
from web3 import Web3

SYMBOL_NODE_URL = os.getenv(
	'SYMBOL_NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using Symbol node {SYMBOL_NODE_URL}')

ETH_RPC_URL = os.getenv('ETH_RPC_URL', 'https://ethereum-sepolia-rpc.publicnode.com')
print(f'Using Ethereum RPC {ETH_RPC_URL}')

# Ethereum HTLC contract on Sepolia
HTLC_ADDRESS = '0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B'
HTLC_ABI = [
	{
		'name': 'newContract',
		'type': 'function',
		'stateMutability': 'payable',
		'inputs': [
			{'name': '_receiver', 'type': 'address'},
			{'name': '_hashlock', 'type': 'bytes32'},
			{'name': '_timelock', 'type': 'uint256'}
		],
		'outputs': [{'name': 'contractId', 'type': 'bytes32'}]
	},
	{
		'name': 'withdraw',
		'type': 'function',
		'stateMutability': 'nonpayable',
		'inputs': [
			{'name': '_contractId', 'type': 'bytes32'},
			{'name': '_preimage', 'type': 'bytes'}
		],
		'outputs': [{'name': '', 'type': 'bool'}]
	},
	{
		'name': 'getContract',
		'type': 'function',
		'stateMutability': 'view',
		'inputs': [
			{'name': '_contractId', 'type': 'bytes32'}
		],
		'outputs': [
			{'name': 'sender', 'type': 'address'},
			{'name': 'receiver', 'type': 'address'},
			{'name': 'amount', 'type': 'uint256'},
			{'name': 'hashlock', 'type': 'bytes32'},
			{'name': 'timelock', 'type': 'uint256'},
			{'name': 'withdrawn', 'type': 'bool'},
			{'name': 'refunded', 'type': 'bool'},
			{'name': 'preimage', 'type': 'bytes'}
		]
	},
	{
		'name': 'LogHTLCNew',
		'type': 'event',
		'inputs': [
			{'name': 'contractId', 'type': 'bytes32', 'indexed': True},
			{'name': 'sender', 'type': 'address', 'indexed': True},
			{'name': 'receiver', 'type': 'address', 'indexed': True},
			{'name': 'amount', 'type': 'uint256', 'indexed': False},
			{'name': 'hashlock', 'type': 'bytes32', 'indexed': False},
			{'name': 'timelock', 'type': 'uint256', 'indexed': False}
		]
	}
]

# Helper function to fetch current Symbol network time
def get_network_time():
	time_path = '/node/time'
	print(f'Fetching current network time from {time_path}')
	with urllib.request.urlopen(
		f'{SYMBOL_NODE_URL}{time_path}'
	) as response:
		response_json = json.loads(response.read().decode())
		timestamp = NetworkTimestamp(int(
			response_json['communicationTimestamps'][
				'receiveTimestamp']))
		print(f'  Network time: {timestamp.timestamp}'
			' ms since nemesis')
		return timestamp


# Helper function to fetch recommended Symbol fee multiplier
def get_fee_multiplier():
	fee_path = '/network/fees/transaction'
	print(f'Fetching recommended fees from {fee_path}')
	with urllib.request.urlopen(
		f'{SYMBOL_NODE_URL}{fee_path}'
	) as response:
		response_json = json.loads(response.read().decode())
		median_mult = response_json['medianFeeMultiplier']
		minimum_mult = response_json['minFeeMultiplier']
		fee_mult = max(median_mult, minimum_mult)
		print(f'  Fee multiplier: {fee_mult}')
		return fee_mult


# Helper function to announce a Symbol transaction
def announce_transaction(payload, endpoint, label):
	print(f'Announcing {label} to {endpoint}')
	request = urllib.request.Request(
		f'{SYMBOL_NODE_URL}{endpoint}',
		data=payload.encode(),
		headers={'Content-Type': 'application/json'},
		method='PUT'
	)
	with urllib.request.urlopen(request) as response:
		print(f'  Response: {response.read().decode()}')


# Helper function to wait for Symbol transaction status
def wait_for_status(hash_value, expected_status, label):
	print(f'Waiting for {label} to reach {expected_status} status...')
	attempts = 0
	max_attempts = 60

	while attempts < max_attempts:
		try:
			url = (f'{SYMBOL_NODE_URL}/transactionStatus/{hash_value}')
			with urllib.request.urlopen(url) as response:
				status = json.loads(response.read().decode())
				print(f'  Transaction status: {status["group"]}')

				if status['group'] == 'failed':
					raise Exception(f'{label} failed: {status["code"]}')

				if status['group'] == expected_status:
					print(f'{label} {expected_status}'
						f' in {attempts} seconds')
					return

		except urllib.error.HTTPError as e:
			if e.code != 404:
				raise
			print('  Transaction status: not yet available')

		attempts += 1
		time.sleep(1)

	raise Exception(
		f'{label} not {expected_status} after {max_attempts} attempts')


# Symbol accounts
facade = SymbolFacade('testnet')

ALICE_PRIVATE_KEY = os.getenv('ALICE_PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
alice_key_pair = SymbolFacade.KeyPair(PrivateKey(ALICE_PRIVATE_KEY))
alice_address = facade.network.public_key_to_address(
	alice_key_pair.public_key)
print(f'Alice Symbol address: {alice_address}')

BOB_PRIVATE_KEY = os.getenv('BOB_PRIVATE_KEY',
	'1111111111111111111111111111111111111111111111111111111111111111')
bob_key_pair = SymbolFacade.KeyPair(PrivateKey(BOB_PRIVATE_KEY))
bob_address = facade.network.public_key_to_address(
	bob_key_pair.public_key)
print(f'Bob Symbol address: {bob_address}')

# Ethereum accounts
w3 = Web3(Web3.HTTPProvider(ETH_RPC_URL))

ALICE_ETH_KEY = os.getenv('ALICE_ETH_PRIVATE_KEY',
	'0xa73276699ba72dc7b5c9d08deaf8cd88eec33c866341b120304432b89586d45d')
alice_eth_account = w3.eth.account.from_key(ALICE_ETH_KEY)
print(f'Alice ETH address: {alice_eth_account.address}')

BOB_ETH_KEY = os.getenv('BOB_ETH_PRIVATE_KEY',
	'0x8e85561005f27d926af79a7ce3e76e75108a09ff2ab78bf65b5578d2e5d509bf')
bob_eth_account = w3.eth.account.from_key(BOB_ETH_KEY)
print(f'Bob ETH address: {bob_eth_account.address}')

try:
	# --- Alice: Generate proof and hashlock ---
	print('\n--- Alice: Generate proof and hashlock ---')

	proof = os.urandom(32)
	print(f'Proof (hex): {proof.hex()}')

	first_hash = hashlib.sha256(proof).digest()
	secret = hashlib.sha256(first_hash).digest()
	print(f'Secret (double SHA-256): {secret.hex()}')

	# --- Alice: Lock ETH on Ethereum ---
	print('\n--- Alice: Lock ETH on Ethereum ---')

	htlc = w3.eth.contract(address=HTLC_ADDRESS, abi=HTLC_ABI)
	timelock = int(time.time()) + 72 * 60 * 60
	print(f'Ethereum timelock (Unix): {timelock}')

	lock_call = htlc.functions.newContract(
		bob_eth_account.address, secret, timelock)
	lock_tx = lock_call.build_transaction({
		'from': alice_eth_account.address,
		'value': w3.to_wei(0.01, 'ether'),
		'nonce': w3.eth.get_transaction_count(alice_eth_account.address)
	})
	signed_lock_tx = alice_eth_account.sign_transaction(lock_tx)
	lock_tx_hash = w3.eth.send_raw_transaction(
		signed_lock_tx.raw_transaction)
	print(f'Lock TX hash: {lock_tx_hash.hex()}')

	lock_receipt = w3.eth.wait_for_transaction_receipt(lock_tx_hash)
	print(f'Lock confirmed in block {lock_receipt.blockNumber}')

	# Extract the contractId from the LogHTLCNew event
	contract_id = lock_receipt.logs[0].topics[1]
	print(f'HTLC contract ID: {contract_id.hex()}')

	# --- Bob: Create secret lock on Symbol ---
	print('\n--- Bob: Create secret lock on Symbol ---')

	# Bob queries the Ethereum contract to get the hashlock
	contract_info = htlc.functions.getContract(contract_id).call()
	hashlock = contract_info[3]  # hashlock field
	print(f'Hashlock from chain: {hashlock.hex()}')

	lock_duration = 5760  # ~48h at 30s blocks
	print(f'Lock duration: {lock_duration} blocks')

	secret_lock_transaction = facade.transaction_factory.create({
		'type': 'secret_lock_transaction_v1',
		'signer_public_key': bob_key_pair.public_key,
		'deadline': get_network_time().add_hours(2).timestamp,
		'recipient_address': alice_address,
		'mosaic': {
			'mosaic_id': generate_mosaic_alias_id('symbol.xym'),
			'amount': 1_000000  # 1 XYM
		},
		'duration': lock_duration,
		'secret': Hash256(hashlock),
		'hash_algorithm': 'hash_256'
	})
	secret_lock_transaction.fee = Amount(
		get_fee_multiplier() * secret_lock_transaction.size)

	# Sign and announce
	lock_signature = facade.sign_transaction(
		bob_key_pair, secret_lock_transaction)
	lock_payload = facade.transaction_factory.attach_signature(
		secret_lock_transaction, lock_signature)

	print('Built secret lock transaction:')
	print(json.dumps(secret_lock_transaction.to_json(), indent=2))

	lock_hash = facade.hash_transaction(secret_lock_transaction)
	print(f'Secret lock transaction hash: {lock_hash}')
	announce_transaction(lock_payload, '/transactions', 'secret lock')
	wait_for_status(lock_hash, 'confirmed', 'Secret lock')

	# --- Alice: Claim XYM on Symbol ---
	print('\n--- Alice: Claim XYM on Symbol ---')

	secret_proof_transaction = facade.transaction_factory.create({
		'type': 'secret_proof_transaction_v1',
		'signer_public_key': alice_key_pair.public_key,
		'deadline': get_network_time().add_hours(2).timestamp,
		'recipient_address': alice_address,
		'secret': Hash256(hashlock),
		'hash_algorithm': 'hash_256',
		'proof': proof
	})
	secret_proof_transaction.fee = Amount(
		get_fee_multiplier() * secret_proof_transaction.size)

	# Sign and announce
	proof_signature = facade.sign_transaction(
		alice_key_pair, secret_proof_transaction)
	proof_payload = facade.transaction_factory.attach_signature(
		secret_proof_transaction, proof_signature)

	print('Built secret proof transaction:')
	print(json.dumps(secret_proof_transaction.to_json(), indent=2))

	proof_hash = facade.hash_transaction(secret_proof_transaction)
	print(f'Secret proof transaction hash: {proof_hash}')
	announce_transaction(proof_payload, '/transactions', 'secret proof')
	wait_for_status(proof_hash, 'confirmed', 'Secret proof')

	# --- Bob: Withdraw ETH on Ethereum ---
	print('\n--- Bob: Withdraw ETH on Ethereum ---')

	# Retrieve the proof from the confirmed transaction
	tx_path = f'/transactions/confirmed/{proof_hash}'
	print(f'Fetching proof from {tx_path}')
	with urllib.request.urlopen(
		f'{SYMBOL_NODE_URL}{tx_path}'
	) as response:
		tx_json = json.loads(response.read().decode())
		revealed_proof = bytes.fromhex(tx_json['transaction']['proof'])
		print(f'Proof from chain: {revealed_proof.hex()}')

	withdraw_call = htlc.functions.withdraw(contract_id, revealed_proof)
	withdraw_tx = withdraw_call.build_transaction({
		'from': bob_eth_account.address,
		'nonce': w3.eth.get_transaction_count(bob_eth_account.address)
	})
	signed_withdraw_tx = bob_eth_account.sign_transaction(withdraw_tx)
	withdraw_tx_hash = w3.eth.send_raw_transaction(
		signed_withdraw_tx.raw_transaction)
	print(f'Withdraw TX hash: {withdraw_tx_hash.hex()}')

	withdraw_receipt = w3.eth.wait_for_transaction_receipt(
		withdraw_tx_hash)
	print(f'Withdraw confirmed in block {withdraw_receipt.blockNumber}')

	print('\n--- Cross-chain swap complete ---')

except urllib.error.URLError as e:
	print(e.reason)
except Exception as e:
	print(e)
