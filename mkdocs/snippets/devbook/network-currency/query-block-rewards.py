import json
import os
import urllib.request

from symbolchain.CryptoTypes import PublicKey
from symbolchain.symbol.Network import Address, Network

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

BLOCK_HEIGHT = os.getenv('BLOCK_HEIGHT', '3222290')

try:
	# Get the block header [>step-1]
	block_url = f'{NODE_URL}/blocks/{BLOCK_HEIGHT}'
	with urllib.request.urlopen(block_url) as response:
		block = json.loads(response.read())
	signer = Network.TESTNET.public_key_to_address(
		PublicKey(block['block']['signerPublicKey']))
	beneficiary = block['block']['beneficiaryAddress']
	print(f'Block height: {BLOCK_HEIGHT}')
	print(f'Signer: {signer}')
	beneficiary_b32 = Address.from_decoded_address_hex_string(
		beneficiary)
	print(f'Beneficiary: {beneficiary_b32}')
	# [<step-1]
	# Get the network sink address [>step-2]
	properties_url = f'{NODE_URL}/network/properties'
	with urllib.request.urlopen(properties_url) as response:
		properties = json.loads(response.read())
	sink_b32 = properties['chain']['harvestNetworkFeeSinkAddress']
	sink = Address(sink_b32).bytes.hex().upper()
	print(f'Network sink: {sink_b32}')
	# [<step-2]
	# Get the inflation reward at this height [>step-3]
	inflation_url = f'{NODE_URL}/network/inflation/at/{BLOCK_HEIGHT}'
	with urllib.request.urlopen(inflation_url) as response:
		inflation = json.loads(response.read())
	reward = int(inflation['rewardAmount'])
	print(f'Inflation reward: {reward / 1e6:,.6f} XYM')
	# [<step-3]
	# Get harvest fee receipts for this block [>step-4]
	receipts_url = (f'{NODE_URL}/statements/transaction'
		f'?height={BLOCK_HEIGHT}'
		f'&receiptType=8515')
	with urllib.request.urlopen(receipts_url) as response:
		receipts = json.loads(response.read())

	# Label and display the reward distribution
	total = 0
	print('\nReward distribution:')
	for item in receipts['data']:
		for r in item['statement']['receipts']:
			if r['type'] != 8515:
				continue
			amount = int(r['amount'])
			total += amount
			target = r['targetAddress']
			if target == sink:
				label = 'Network sink (5%)'
			elif target == beneficiary:
				label = 'Beneficiary (25%)'
			else:
				label = 'Harvester'
				print(f'  {label}: {amount / 1e6:,.6f} XYM')
				harvester = Address.from_decoded_address_hex_string(
					target)
				print(f'  Harvester: {harvester}')
				continue
			print(f'  {label}: {amount / 1e6:,.6f} XYM')
	# [<step-4]
	# Summary [>step-5]
	fees = total - reward
	print('\nSummary:')
	print(f'  Total block reward: {total / 1e6:,.6f} XYM')
	print(f'  Inflation: {reward / 1e6:,.6f} XYM')
	print(f'  Transaction fees: {fees / 1e6:,.6f} XYM')
	# [<step-5]
except Exception as error:
	print(error)
