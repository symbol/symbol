import json
import os
import urllib.request

from symbolchain.symbol.IdGenerator import generate_namespace_path
from symbolchain.symbol.Network import Address

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

NAMESPACE_NAME = os.getenv('NAMESPACE_NAME', 'symbol.xym')
print(f'Namespace name: {NAMESPACE_NAME}')

try:
	# Generate namespace ID from name
	path = generate_namespace_path(NAMESPACE_NAME)
	namespace_id = path[-1]
	namespace_id_hex = f'{namespace_id:x}'
	print(f'Namespace ID: {namespace_id} (0x{namespace_id_hex})')

	# Fetch namespace information
	namespace_path = f'/namespaces/{namespace_id_hex}'
	print(f'Fetching namespace information from {namespace_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{namespace_path}') as response:
		response_json = json.loads(response.read().decode())
		ns = response_json['namespace']
		print('Namespace information:')
		reg_type = ns['registrationType']
		print(f'  Registration type: {reg_type}')
		owner_address = Address.from_decoded_address_hex_string(
			ns['ownerAddress'])
		print(f'  Owner address: {owner_address}')
		depth = int(ns['depth'])
		print(f'  Depth: {depth}')
		print(f'  Level 0 ID: {ns["level0"]}')
		if depth >= 2:
			print(f'  Level 1 ID: {ns["level1"]}')
		if depth == 3 and 'level2' in ns:
			print(f'  Level 2 ID: {ns["level2"]}')
		print(f'  Start height: {ns["startHeight"]}')
		end_height = int(ns['endHeight'])
		print(f'  End height: {end_height} (0x{end_height:X})')

		# Display alias information
		alias = ns['alias']
		alias_type = alias['type']
		print(f'  Alias type: {alias_type}')
		if alias_type == 1:
			print(f'  Linked mosaic ID: {alias["mosaicId"]}')
		elif alias_type == 2:
			linked_address = (
				Address.from_decoded_address_hex_string(
					alias['address']))
			print(f'  Linked address: {linked_address}')
		else:
			print('  No alias linked')

except Exception as e:
	print(e)
