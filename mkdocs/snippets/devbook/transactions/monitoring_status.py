import json
import os
import time
import urllib.request

# Configuration
NODE_URL = os.getenv("NODE_URL", "https://reference.symboltest.net:3001")
print(f'Using node {NODE_URL}')

# Transaction hash to monitor
transaction_hash = os.getenv("TRANSACTION_HASH",  # [>step-1]
	"2B6D3B5232E06B9D32682F518C765301FCF9716BFA1EEEF9523653406E04C7EA")
# [<step-1]
print(f"Monitoring transaction: {transaction_hash}")


def wait_for_transaction_confirmation(  # [>step-2]
	transaction_hash, max_attempts=60, wait_seconds=2
):
	"""
	Poll the transaction status endpoint until the transaction
	is confirmed.

	Args:
		transaction_hash: The hash of the transaction to monitor
		max_attempts: Maximum number of polling attempts for confirmation
		wait_seconds: Seconds to wait between attempts

	Returns:
		True if transaction was confirmed
	"""
	status_path = f"/transactionStatus/{transaction_hash}"
	print(f"\nWaiting for transaction confirmation")
	print(f"Polling {status_path}")

	for attempt in range(1, max_attempts + 1):
		try:
			# Query the transaction status endpoint
			url = f"{NODE_URL}{status_path}"
			with urllib.request.urlopen(url) as response:
				response_json = json.loads(response.read().decode())

				# Parse the response
				status_group = response_json["group"]
				status_code = response_json["code"]
				status_hash = response_json["hash"]
				status_deadline = response_json["deadline"]

				print(f"  Attempt {attempt}:")
				print(f"    Status: {status_group}")
				print(f"    Code: {status_code}")
				print(f"    Hash: {status_hash}")
				print(f"    Deadline: {status_deadline}")
				# [<step-2]
				# Check if the transaction has been confirmed [>step-3]
				if status_group == "confirmed":
					print(f"\nTransaction confirmed!")
					return True
				# [<step-3]
				# Check if the transaction failed [>step-4]
				if status_group == "failed":
					print(
						f"\nTransaction failed with code: {status_code}"
					)
					raise RuntimeError(
						f"Transaction failed: {status_code}"
					)  # [<step-4]
		# [>step-5]
		except Exception as e:
			if hasattr(e, 'code') and e.code == 404:
				print(
					f"  Attempt {attempt}: Transaction status not "
					"yet available"
				)
			else:
				raise
		# [<step-5]
		# Wait before next attempt (except on last attempt) [>step-6]
		if attempt < max_attempts:
			time.sleep(wait_seconds)  # [<step-6]
	# [>step-7]
	print(f"\nTransaction not confirmed after {max_attempts} attempts")
	raise RuntimeError(
		f"Transaction {transaction_hash} not confirmed in time"
	)  # [<step-7]


# Monitor the transaction until it's confirmed
wait_for_transaction_confirmation(transaction_hash)
