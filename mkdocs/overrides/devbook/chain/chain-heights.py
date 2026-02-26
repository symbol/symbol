import json
import os
import time
import urllib.request

NODE_URL = os.getenv(
	'NODE_URL', 'https://reference.symboltest.net:3001')
print(f'Using node {NODE_URL}')

prev_height = None
prev_finalized_height = None
height_changed_at = None
finalized_changed_at = None

try:
	while True:
		with urllib.request.urlopen(f'{NODE_URL}/chain/info') as response:
			chain_info = json.loads(response.read().decode())

		height = int(chain_info['height'])
		finalized = chain_info['latestFinalizedBlock']
		finalized_height = int(finalized['height'])

		now = time.time()

		if prev_height is not None and height != prev_height:
			height_changed_at = now
		if (
			prev_finalized_height is not None
			and finalized_height != prev_finalized_height
		):
			finalized_changed_at = now

		if height_changed_at is not None:
			h_ago = f"{int(now - height_changed_at)}s ago"
		else:
			h_ago = "-"
		if finalized_changed_at is not None:
			f_ago = f"{int(now - finalized_changed_at)}s ago"
		else:
			f_ago = "-"

		print(
			f"Height: {height:>10,}"
			f"  (changed {h_ago})"
			f"  |  Finalized: {finalized_height:>10,}"
			f"  (changed {f_ago})"
		)

		prev_height = height
		prev_finalized_height = finalized_height
		time.sleep(1)

except Exception as error:
	print(error)
