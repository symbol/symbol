import hashlib
import json
import os
import urllib.request
from binascii import unhexlify

from symbolchain.BufferWriter import BufferWriter
from symbolchain.CryptoTypes import Hash256
from symbolchain.symbol.Merkle import (
	deserialize_patricia_tree_nodes,
	prove_patricia_merkle
)

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

try:
	# Fetch the network currency mosaic ID [>step-1]
	url = f'{NODE_URL}/network/properties'
	with urllib.request.urlopen(url) as response:
		props = json.loads(response.read().decode())
	raw_id = props['chain']['currencyMosaicId']
	mosaic_id = int(raw_id.replace("'", ""), 16)
	mosaic_id_hex = f'{mosaic_id:016X}'
	print(f'Currency mosaic ID: {mosaic_id_hex}')

	# Fetch the mosaic properties
	mosaic_path = f'/mosaics/{mosaic_id_hex}'
	print(f'Fetching mosaic from {mosaic_path}')
	url = f'{NODE_URL}{mosaic_path}'
	with urllib.request.urlopen(url) as response:
		mosaic_data = json.loads(response.read().decode())
	mosaic = mosaic_data['mosaic']
	print(json.dumps(mosaic, indent=2))
	# [<step-1]
	# Serialize and hash the mosaic properties [>step-2]
	writer = BufferWriter()
	writer.write_int(int(mosaic['version']), 2)
	writer.write_int(int(mosaic['id'], 16), 8)
	writer.write_int(int(mosaic['supply']), 8)
	writer.write_int(int(mosaic['startHeight']), 8)
	writer.write_bytes(unhexlify(mosaic['ownerAddress']))
	writer.write_int(int(mosaic['revision']), 4)
	writer.write_int(int(mosaic['flags']), 1)
	writer.write_int(int(mosaic['divisibility']), 1)
	writer.write_int(int(mosaic['duration']), 8)
	hashed_value = Hash256(hashlib.sha3_256(writer.buffer).digest())
	print(f'Hashed value: {hashed_value}')

	# Hash the mosaic ID to get the encoded key
	writer = BufferWriter()
	writer.write_int(int(mosaic['id'], 16), 8)
	encoded_key = Hash256(hashlib.sha3_256(writer.buffer).digest())
	print(f'Encoded key: {encoded_key}')
	# [<step-2]
	# Fetch the current network height [>step-3]
	url = f'{NODE_URL}/chain/info'
	with urllib.request.urlopen(url) as response:
		chain_info = json.loads(response.read().decode())
	height = int(chain_info['height'])
	print(f'Current height: {height}')

	# Fetch the block's state hash and roots
	block_path = f'/blocks/{height}'
	print(f'Fetching block from {block_path}')
	url = f'{NODE_URL}{block_path}'
	with urllib.request.urlopen(url) as response:
		block_data = json.loads(response.read().decode())
	state_hash = Hash256(block_data['block']['stateHash'])
	sub_cache_key = 'stateHashSubCacheMerkleRoots'
	roots = [Hash256(r) for r in block_data['meta'][sub_cache_key]]
	print(f'State hash: {state_hash}')
	# [<step-3]
	# Fetch the patricia tree path [>step-4]
	tree_url = f'/mosaics/{mosaic_id_hex}/merkle'
	print(f'Fetching tree path from {tree_url}')
	url = f'{NODE_URL}{tree_url}'
	with urllib.request.urlopen(url) as response:
		tree_data = json.loads(response.read().decode())
	merkle_path = deserialize_patricia_tree_nodes(
		unhexlify(tree_data['raw']))
	print(f'Tree path: {len(merkle_path)} nodes')
	key_hex = str(encoded_key)
	key_pos = 0
	for i, node in enumerate(merkle_path):
		key_pos += node.path.size
		path_str = (f'  path: {node.hex_path}' if node.path.size else '')
		if hasattr(node, 'value'):
			print(f'  [{i}] leaf{path_str}  value: {node.value}')
		else:
			nibble = key_hex[key_pos]
			key_pos += 1
			active = [
				f'{j:X}' for j, link in enumerate(node.links) if link
			]
			print(f'  [{i}] branch{path_str}'
				f'  links: [{",".join(active)}]'
				f'  -> follow {nibble}')
	# [<step-4]
	# Verify the mosaic state [>step-5]
	result = prove_patricia_merkle(
		encoded_key, hashed_value, merkle_path, state_hash, roots)

	if result.name == 'VALID_POSITIVE':
		print(f'Mosaic {mosaic_id_hex} state verified at height {height}')
	else:
		raise RuntimeError(
			f'Mosaic {mosaic_id_hex} proof failed: {result.name}')
	# [<step-5]
except Exception as e:
	print(e)
