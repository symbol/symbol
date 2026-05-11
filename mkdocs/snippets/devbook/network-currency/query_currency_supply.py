import os
import urllib.request

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

SUPPLY_URL = f'{NODE_URL}/network/currency/supply'

try:
	with urllib.request.urlopen(f'{SUPPLY_URL}/max') as response:  # [>step-1]
		maximum = float(response.read().decode().strip())
	print(f'Maximum supply: {maximum:,.6f} XYM')

	with urllib.request.urlopen(f'{SUPPLY_URL}/total') as response:
		total = float(response.read().decode().strip())
	print(f'Total supply: {total:,.6f} XYM')

	with urllib.request.urlopen(f'{SUPPLY_URL}/circulating') as response:
		circulating = float(response.read().decode().strip())
	print(f'Circulating supply: {circulating:,.6f} XYM')  # [<step-1]
	# [>step-2]
	non_circulating = total - circulating
	print(f'Non-circulating: {non_circulating:,.6f} XYM')

	unminted = maximum - total
	print(f'Unminted: {unminted:,.6f} XYM')
	# [<step-2]
except Exception as error:
	print(error)
