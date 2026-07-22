import os
import urllib.request

NODE_URL = os.getenv('NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

SUPPLY_URL = f'{NODE_URL}/network/currency/supply'

try:
	with urllib.request.urlopen(f'{SUPPLY_URL}/max') as response:  # [>step-1]
		maximum_supply = float(response.read().decode().strip())
	print(f'Maximum supply: {maximum_supply:,.6f} XYM')

	with urllib.request.urlopen(f'{SUPPLY_URL}/total') as response:
		total_supply = float(response.read().decode().strip())
	print(f'Total supply: {total_supply:,.6f} XYM')

	with urllib.request.urlopen(f'{SUPPLY_URL}/circulating') as response:
		circulating_supply = float(response.read().decode().strip())
	print(f'Circulating supply: {circulating_supply:,.6f} XYM')  # [<step-1]
	# [>step-2]
	non_circulating_supply = total_supply - circulating_supply
	print(f'Non-circulating supply: {non_circulating_supply:,.6f} XYM')

	unminted_supply = maximum_supply - total_supply
	print(f'Unminted supply: {unminted_supply:,.6f} XYM')
	# [<step-2]
except Exception as error:
	print(error)
