import json
import os
import urllib.request

from symbolchain.CryptoTypes import Hash256
from symbolchain.symbol.Merkle import MerklePart, prove_merkle

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

TX_HASH = os.getenv('TRANSACTION_HASH',
	'99011A8DBC086E0C359E9D8A38FEC6714C33726FCD0C1B5C0F772A82400D808B')
print(f'Transaction hash: {TX_HASH}')

try:
	# Fetch the confirmed transaction to get its block height [>step-1]
	tx_path = f'/transactions/confirmed/{TX_HASH}'
	print(f'Fetching transaction from {tx_path}')
	with urllib.request.urlopen(f'{NODE_URL}{tx_path}') as response:
		tx_data = json.loads(response.read().decode())

	print(json.dumps(tx_data['meta'], indent=2))
	block_height = tx_data['meta']['height']
	merkle_component_hash = Hash256(
		tx_data['meta']['merkleComponentHash'])
	# [<step-1]
	# Fetch the block header to get the transactions hash [>step-2]
	block_path = f'/blocks/{block_height}'
	print(f'Fetching block from {block_path}')
	with urllib.request.urlopen(f'{NODE_URL}{block_path}') as response:
		block_data = json.loads(response.read().decode())

	print(json.dumps({
		'height': block_data['block']['height'],
		'transactionsHash': block_data['block']['transactionsHash'],
	}, indent=2))
	transactions_hash = Hash256(
		block_data['block']['transactionsHash'])
	# [<step-2]
	# Fetch the merkle proof path for the transaction [>step-3]
	merkle_path = (f'/blocks/{block_height}'
		f'/transactions/{TX_HASH}/merkle')
	print('Fetching merkle proof:')
	print(f'  {merkle_path}')
	with urllib.request.urlopen(f'{NODE_URL}{merkle_path}') as response:
		merkle_data = json.loads(response.read().decode())

	print(json.dumps(merkle_data, indent=2))

	# Convert the API response to the format expected by the SDK
	merkle_proof_path = [
		MerklePart(Hash256(part['hash']), part['position'] == 'left')
		for part in merkle_data['merklePath']
	]
	print(f'  Merkle path length: {len(merkle_proof_path)}')
	# [<step-3]
	# Verify that the transaction is included in the block [>step-4]
	is_proven = prove_merkle(
		merkle_component_hash, merkle_proof_path,
		transactions_hash)

	if is_proven:
		print(
			f'Transaction {TX_HASH[:16]}...'
			f' proven in block {block_height}')
	else:
		raise RuntimeError(
			f'Transaction {TX_HASH[:16]}...'
			f' NOT proven in block {block_height}')
	# [<step-4]
except Exception as e:
	print(e)
