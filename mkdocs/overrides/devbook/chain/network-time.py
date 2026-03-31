import json
import os
import datetime
import urllib.request

NODE_URL = os.getenv(
	'NODE_URL', 'https://whydah.symbolmain.net:3001')
print(f'Using node {NODE_URL}')

try:
	# Fetch Nemesis timestamp
	properties_path = '/network/properties'
	print(f'Fetching network properties from {properties_path}')
	with urllib.request.urlopen(
			f'{NODE_URL}{properties_path}') as response:
		response_json = json.loads(response.read().decode())
		nemesis_datetime = datetime.datetime.fromtimestamp(
			int(response_json['network']['epochAdjustment'].rstrip('s')),
			tz=datetime.timezone.utc)

	# Fetch current network timestamp
	time_path = '/node/time'
	print(f'Fetching current network time from {time_path}')
	with urllib.request.urlopen(f'{NODE_URL}{time_path}') as response:
		response_json = json.loads(response.read().decode())
		network_ms = int(
			response_json['communicationTimestamps']['receiveTimestamp'])

	network_datetime = nemesis_datetime + datetime.timedelta(
		milliseconds=network_ms)

	print(f'\nNemesis time (UTC): {nemesis_datetime}')
	print(f'Network time (ms since Nemesis): {network_ms}')
	print(f'Network time (UTC): {network_datetime}')
except Exception as error:
	print(error)
