import json
import os
import urllib.request

NODE_URL = os.environ.get(
	'NODE_URL', 'https://001-sai-dual.symboltest.net:3001')
print(f'Using node {NODE_URL}')


def get_account_info(account_identifier):
	"""
	Fetch account information by address or public key.

	Args:
		account_identifier: The account address or public key

	Returns:
		Dictionary containing the account information
	"""
	account_path = f'/accounts/{account_identifier}'
	with urllib.request.urlopen(f'{NODE_URL}{account_path}') as response:
		account_info = json.loads(response.read().decode())
		return account_info['account']


def get_mosaic_names(mosaic_ids):
	"""
	Fetch friendly names for a set of mosaics.

	Args:
		mosaic_ids: List of mosaic IDs as integers

	Returns:
		Dictionary mapping mosaic IDs to their namespace names
	"""
	mosaic_ids_hex = [f'{mosaic_id:016X}' for mosaic_id in mosaic_ids]
	request_body = json.dumps({'mosaicIds': mosaic_ids_hex}).encode()
	request = urllib.request.Request(
		f'{NODE_URL}/namespaces/mosaic/names',
		data=request_body,
		headers={'Content-Type': 'application/json'}
	)
	with urllib.request.urlopen(request) as response:
		names_info = json.loads(response.read().decode())
		# Build a dictionary mapping mosaic IDs to their names
		names_map = {}
		for entry in names_info['mosaicNames']:
			mosaic_id = int(entry['mosaicId'], 16)
			names_map[mosaic_id] = entry['names']
		return names_map


def get_mosaics_info(mosaic_ids):
	"""
	Fetch information for multiple mosaics in a single request.

	Args:
		mosaic_ids: List of mosaic IDs as integers

	Returns:
		Dictionary mapping mosaic IDs to their properties
	"""
	mosaic_ids_hex = [f'{mosaic_id:016X}' for mosaic_id in mosaic_ids]
	request_body = json.dumps({'mosaicIds': mosaic_ids_hex}).encode()
	request = urllib.request.Request(
		f'{NODE_URL}/mosaics',
		data=request_body,
		headers={'Content-Type': 'application/json'}
	)
	with urllib.request.urlopen(request) as response:
		mosaics_info = json.loads(response.read().decode())
		# Build a dictionary mapping mosaic IDs to their properties
		mosaics_map = {}
		for entry in mosaics_info:
			mosaic_id = int(entry['mosaic']['id'], 16)
			mosaics_map[mosaic_id] = entry['mosaic']
		return mosaics_map


def format_amount(amount, divisibility):
	"""
	Format an atomic amount with decimal places.

	Args:
		amount: The atomic amount as an integer
		divisibility: Number of decimal places

	Returns:
		Formatted amount as a string
	"""
	if divisibility == 0:
		return str(amount)
	whole_part = amount // (10 ** divisibility)
	fractional_part = amount % (10 ** divisibility)
	return f'{whole_part}.{fractional_part:0{divisibility}d}'


# The account address to query
ADDRESS = os.environ.get(
	'ADDRESS', 'TBIL6D6RURP45YQRWV6Q7YVWIIPLQGLZQFHWFEQ')
print(f'Fetching account information from {ADDRESS}')

# Get account information
account = get_account_info(ADDRESS)

# Display balances for all mosaics the account holds
account_mosaics = account['mosaics']
if not account_mosaics:
	print('Account holds no mosaics')
else:
	print(f'Account holds {len(account_mosaics)} mosaic(s):')

# Fetch mosaic properties and names for all mosaics
mosaic_ids = [int(m['id'], 16) for m in account_mosaics]
mosaic_names = get_mosaic_names(mosaic_ids)
mosaics_info = get_mosaics_info(mosaic_ids)

for mosaic_entry in account_mosaics:
	mosaic_id = int(mosaic_entry['id'], 16)
	balance = int(mosaic_entry['amount'])

	# Get mosaic properties
	info = mosaics_info[mosaic_id]
	divisibility = info['divisibility']

	# Format and display the balance
	formatted_balance = format_amount(balance, divisibility)
	mosaic_id_hex = f'0x{mosaic_id:016X}'

	# Display mosaic ID and names (if available)
	names = mosaic_names.get(mosaic_id, [])
	if names:
		names_str = ', '.join(names)
		print(f'- Mosaic {mosaic_id_hex} ({names_str})')
	else:
		print(f'- Mosaic {mosaic_id_hex}')

	print(f'  Balance: {formatted_balance}')
	print(f'  Balance (atomic): {balance}')
	print(f'  Divisibility: {divisibility}')
