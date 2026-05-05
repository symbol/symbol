import json
import os
import urllib.request

from symbolchain.sc import MosaicFlags

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

MOSAIC_ID = os.getenv('MOSAIC_ID', '72C0212E67A08BCE')
print(f'Mosaic ID: {MOSAIC_ID}')

try:
	# Fetch mosaic information [>step-1]
	mosaic_path = f'/mosaics/{MOSAIC_ID}'
	print(f'Fetching mosaic information from {mosaic_path}')
	with urllib.request.urlopen(f'{NODE_URL}{mosaic_path}') as response:
		response_json = json.loads(response.read().decode())
		mosaic = response_json['mosaic']
		print('Mosaic information:')
		print(f'  Mosaic ID: {mosaic["id"]}')
		print(f'  Supply: {mosaic["supply"]}')
		divisibility = mosaic['divisibility']
		print(f'  Divisibility: {divisibility}')
		flags = MosaicFlags(mosaic['flags'])
		print(f'  Flags: {flags.value} ({flags.name.lower()})')
		print(f'  Duration: {mosaic["duration"]}')
		print(f'  Start height: {mosaic["startHeight"]}')
		print(f'  Revision: {mosaic["revision"]}')
	# [<step-1]
	# Display formatted supply [>step-2]
	supply = int(mosaic['supply'])
	whole = supply // (10 ** divisibility)
	fractional = supply % (10 ** divisibility)
	formatted = f'{whole}.{fractional:0{divisibility}d}'
	print(f'\nSupply in whole units: {formatted}')
	# [<step-2]
	# Fetch namespace names linked to the mosaic [>step-3]
	print(f'\nFetching namespace names for mosaic {MOSAIC_ID}')
	request_body = json.dumps({'mosaicIds': [MOSAIC_ID]}).encode()
	request = urllib.request.Request(
		f'{NODE_URL}/namespaces/mosaic/names',
		data=request_body,
		headers={'Content-Type': 'application/json'}
	)
	with urllib.request.urlopen(request) as response:
		names_info = json.loads(response.read().decode())
		for entry in names_info['mosaicNames']:
			names = entry['names']
			if names:
				print(f'  Namespace aliases: {", ".join(names)}')
			else:
				print('  No namespace aliases linked')
	# [<step-3]
except Exception as e:
	print(e)
