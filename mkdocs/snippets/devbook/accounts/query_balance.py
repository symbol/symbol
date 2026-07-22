import json
import os
import urllib.request

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')


def get_account_info(account_identifier):  # [>step-1]
	"""
	Fetch account information by address or public key.

	Args:
		account_identifier: The account address or public key

	Returns:
		Dictionary containing the account information
	"""
	account_path = f'/accounts/{account_identifier}'
	try:
		account_url = f'{NODE_URL}{account_path}'
		with urllib.request.urlopen(account_url) as response:
			account_info = json.loads(response.read().decode())
			return account_info['account']
	except urllib.error.HTTPError as err:
		if err.status == 404:
			print(f'Address does not exist: {err}')
		elif err.status == 409:
			print(f'Address is not properly formatted: {err}')
		else:
			print(f'Unexpected error: {err}')
		raise SystemExit(1) from err  # [<step-1]


def get_mosaic_names(mosaic_ids):  # [>step-2]
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
			entry_id = int(entry['mosaicId'], 16)
			names_map[entry_id] = entry['names']
		return names_map  # [<step-2]


def get_mosaics_info(mosaic_ids):  # [>step-3]
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
			entry_id = int(entry['mosaic']['id'], 16)
			mosaics_map[entry_id] = entry['mosaic']
		return mosaics_map  # [<step-3]


def format_amount(amount, divisibility):  # [>step-4]
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
	return f'{whole_part}.{fractional_part:0{divisibility}d}'  # [<step-4]


# The account address to query [>step-5]
ADDRESS = os.getenv(
	'ADDRESS', 'TBIL6D6RURP45YQRWV6Q7YVWIIPLQGLZQFHWFEQ')
print(f'Fetching account information from {ADDRESS}')

try:
	# Get account information
	account = get_account_info(ADDRESS)

	# Display balances for all mosaics the account holds
	account_mosaics = account['mosaics']
	if not account_mosaics:
		print('Account holds no mosaics')
	else:
		print(f'Account holds {len(account_mosaics)} mosaic(s):')

		# Fetch mosaic properties and names for all mosaics
		acc_mosaic_ids = [int(m['id'], 16) for m in account_mosaics]
		acc_mosaic_names = get_mosaic_names(acc_mosaic_ids)
		acc_mosaics_info = get_mosaics_info(acc_mosaic_ids)

		for mosaic_entry in account_mosaics:
			mosaic_id = int(mosaic_entry['id'], 16)
			balance = int(mosaic_entry['amount'])

			# Get mosaic properties
			info = acc_mosaics_info[mosaic_id]
			mosaic_divisibility = info['divisibility']

			# Format and display the balance
			formatted_balance = format_amount(balance, mosaic_divisibility)
			mosaic_id_hex = f'0x{mosaic_id:016X}'

			# Display mosaic ID and names (if available)
			names = acc_mosaic_names.get(mosaic_id, [])
			if names:
				names_str = ', '.join(names)
				print(f'- Mosaic {mosaic_id_hex} ({names_str})')
			else:
				print(f'- Mosaic {mosaic_id_hex}')

			print(f'  Balance: {formatted_balance}')
			print(f'  Balance (atomic): {balance}')
			print(f'  Divisibility: {mosaic_divisibility}')
except urllib.error.URLError as e:
	print(e.reason)  # [<step-5]
