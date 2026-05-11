import json
import os
import urllib.request

from symbolchain.symbol.IdGenerator import is_mosaic_alias
from symbolchain.symbol.Network import Address

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

# Hash of a confirmed tx that used a namespace alias [>step-1]
TX_HASH = os.getenv('TRANSACTION_HASH',
	'BA0C65DB752A3BF1B25285540642537ECE8C2CA716577EDF8BF0F8597A85ADC4')
print(f'Transaction hash: {TX_HASH}')
# [<step-1]
try:
	# Retrieve the confirmed transaction [>step-2]
	tx_path = f'/transactions/confirmed/{TX_HASH}'
	print(f'Fetching transaction from {tx_path}')
	with urllib.request.urlopen(f'{NODE_URL}{tx_path}') as response:
		tx_data = json.loads(response.read().decode())

	block_height = tx_data['meta']['height']
	print(f'  Block height: {block_height}')

	# primaryId is 1-based, meta.index is 0-based
	tx_index = int(tx_data['meta']['index'])
	tx_primary = tx_index + 1
	print(f'  Transaction index: {tx_index} (primaryId: {tx_primary})')  # [<step-2]
	# [>step-3]
	recipient_hex = tx_data['transaction']['recipientAddress']
	recipient_bytes = bytes.fromhex(recipient_hex)
	is_address_alias = (recipient_bytes[0] & 0x01) == 1
	print(f'  Recipient: {recipient_hex}')
	print(f'  Is address alias: {is_address_alias}')  # [<step-3]
	# [>step-4]
	aliased_mosaics = set()
	mosaics = tx_data['transaction']['mosaics']
	for mosaic in mosaics:
		mosaic_id = int(mosaic['id'], 16)
		is_alias = is_mosaic_alias(mosaic_id)
		if is_alias:
			aliased_mosaics.add(mosaic['id'])
		print(f'  Mosaic: {mosaic["id"]}')
		print(f'  Is mosaic alias: {is_alias}')
	# [<step-4]
	# Query address resolution statements
	if is_address_alias:  # [>step-5]
		address_path = ('/statements/resolutions/address'
			f'?height={block_height}')
		print(f'\nFetching address resolutions from {address_path}')
		address_url = f'{NODE_URL}{address_path}'
		with urllib.request.urlopen(address_url) as response:
			address_data = json.loads(response.read().decode())

		address_statements = address_data['data']
		print(f'  Found {len(address_statements)}'
			+ ' resolution statement(s)')  # [<step-5]
		# [>step-6]
		for item in address_statements:
			statement = item['statement']
			if statement['unresolved'] != recipient_hex:
				continue
			resolved = None
			for entry in statement['resolutionEntries']:
				source = entry['source']
				if source['primaryId'] <= tx_primary:
					resolved = entry['resolved']
			if resolved:
				address = Address.from_decoded_address_hex_string(resolved)
				print('\nAddress resolution:')
				print(f'  Unresolved:  {statement["unresolved"]}')
				print(f'  Resolved:   {address}')
		# [<step-6]
	# Query mosaic resolution statements
	if len(aliased_mosaics):  # [>step-7]
		mosaic_path = ('/statements/resolutions/mosaic'
			f'?height={block_height}')
		print(f'\nFetching mosaic resolutions from {mosaic_path}')
		mosaic_url = f'{NODE_URL}{mosaic_path}'
		with urllib.request.urlopen(mosaic_url) as response:
			mosaic_data = json.loads(response.read().decode())

		mosaic_statements = mosaic_data['data']
		print(f'  Found {len(mosaic_statements)} resolution statement(s)')

		for item in mosaic_statements:
			statement = item['statement']
			if statement['unresolved'] not in aliased_mosaics:
				continue
			resolved = None
			for entry in statement['resolutionEntries']:
				source = entry['source']
				if source['primaryId'] <= tx_primary:
					resolved = entry['resolved']
			if resolved:
				print('\nMosaic resolution:')
				print(f'  Unresolved: {statement["unresolved"]}')
				print(f'  Resolved:   {resolved}')
	# [<step-7]
except Exception as e:
	print(e)
