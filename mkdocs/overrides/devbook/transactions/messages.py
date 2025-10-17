import json
import os
import time
import urllib.error
import urllib.request
from binascii import hexlify

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.MessageEncoder import MessageEncoder
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.sc import Amount

# Configuration
NODE_URL = os.environ.get(
	"NODE_URL", "https://001-sai-dual.symboltest.net:3001"
)


# Helper function to poll for confirmed transaction
def wait_for_confirmation(hash_value, label):
	print(f"Polling for {label} confirmation...")
	confirmed = False
	attempts = 0
	max_attempts = 60

	while not confirmed and attempts < max_attempts:
		try:
			url = f"{NODE_URL}/transactions/confirmed/{hash_value}"
			with urllib.request.urlopen(url) as response:
				confirmed = True
				print(f"  {label} confirmed!")
				return json.loads(response.read().decode())
		except urllib.error.HTTPError:
			# Transaction not yet confirmed
			pass
		attempts += 1
		time.sleep(2)

	if not confirmed:
		raise Exception(
			f"{label} not confirmed after {max_attempts} attempts"
		)


print(f"Using node {NODE_URL}")

# Set up sender and recipient accounts
facade = SymbolFacade("testnet")

sender_private_key_string = os.environ.get(
	"SENDER_PRIVATE_KEY",
	"0000000000000000000000000000000000000000000000000000000000000000",
)
sender_key_pair = facade.KeyPair(
	PrivateKey(sender_private_key_string)
)
sender_address = facade.network.public_key_to_address(
	sender_key_pair.public_key
)

recipient_private_key_string = os.environ.get(
	"RECIPIENT_PRIVATE_KEY",
	"1111111111111111111111111111111111111111111111111111111111111111",
)
recipient_key_pair = facade.KeyPair(
	PrivateKey(recipient_private_key_string)
)
recipient_address = facade.network.public_key_to_address(
	recipient_key_pair.public_key
)

print(f"Sender address: {sender_address}")
print(f"Recipient address: {recipient_address}\n")

# ===== PLAIN TEXT MESSAGE =====
print("=== Sending Plain Text Message ===")

# Fetch current network time
time_path = "/node/time"
print(f"Fetching current network time from {time_path}")
with urllib.request.urlopen(f"{NODE_URL}{time_path}") as response:
	response_json = json.loads(response.read().decode())
	timestamp = NetworkTimestamp(int(
		response_json['communicationTimestamps']['receiveTimestamp'])
	)
	print(f"  Network time: {timestamp.timestamp} ms since nemesis")

# Fetch recommended fees
fee_path = "/network/fees/transaction"
print(f"Fetching recommended fees from {fee_path}")
with urllib.request.urlopen(f"{NODE_URL}{fee_path}") as response:
	response_json = json.loads(response.read().decode())
	median_mult = response_json["medianFeeMultiplier"]
	minimum_mult = response_json["minFeeMultiplier"]
	fee_mult = max(median_mult, minimum_mult)
	print(f"  Fee multiplier: {fee_mult}")

# Create a plain text message
plain_message = "Hello, Symbol!".encode("utf-8")
print(f"Plain message: {plain_message.decode('utf-8')}")

# Build transfer transaction with plain message
plain_transaction = facade.transaction_factory.create(
	{
		"type": "transfer_transaction_v1",
		"signer_public_key": sender_key_pair.public_key,
		"deadline": timestamp.add_hours(2).timestamp,
		"recipient_address": recipient_address,
		"mosaics": [],
		"message": plain_message,
	}
)
plain_transaction.fee = Amount(fee_mult * plain_transaction.size)

# Sign and announce the transaction
plain_signature = facade.sign_transaction(
	sender_key_pair, plain_transaction
)
plain_json_payload = facade.transaction_factory.attach_signature(
	plain_transaction, plain_signature
)
plain_transaction_hash = facade.hash_transaction(
	plain_transaction
)
print(f"Transaction hash: {plain_transaction_hash}")

plain_announce_request = urllib.request.Request(
	f"{NODE_URL}/transactions",
	data=plain_json_payload.encode("utf-8"),
	headers={"Content-Type": "application/json"},
	method="PUT",
)
with urllib.request.urlopen(plain_announce_request) as response:
	print(f"Plain message transaction announced\n")

# ===== RECEIVING PLAIN TEXT MESSAGE =====
print("=== Receiving Plain Text Message ===")

# Wait for confirmation
plain_tx_data = wait_for_confirmation(
	plain_transaction_hash, "Plain message transaction"
)

# Decode plain message from confirmed transaction
received_plain_message = bytes.fromhex(
	plain_tx_data["transaction"]["message"]
)
print(
	f"Received plain message: {received_plain_message.decode('utf-8')}\n"
)

# ===== ENCRYPTED MESSAGE =====
print("=== Sending Encrypted Message ===")

# Create a message encoder with sender's key pair
sender_message_encoder = MessageEncoder(sender_key_pair)

# Encrypt the message using recipient's public key
secret_message = "This is a secret message!".encode("utf-8")
encrypted_payload = sender_message_encoder.encode(
	recipient_key_pair.public_key, secret_message
)
print(f"Original message: {secret_message.decode('utf-8')}")
print(
	"Encrypted payload: "
	+ hexlify(encrypted_payload).decode("utf-8")
)

# Build transfer transaction with encrypted message
encrypted_transaction = facade.transaction_factory.create(
	{
		"type": "transfer_transaction_v1",
		"signer_public_key": sender_key_pair.public_key,
		"deadline": timestamp.add_hours(2).timestamp,
		"recipient_address": recipient_address,
		"mosaics": [],
		"message": encrypted_payload,
	}
)
encrypted_transaction.fee = Amount(fee_mult * encrypted_transaction.size)

# Sign and announce the transaction
encrypted_signature = facade.sign_transaction(
	sender_key_pair, encrypted_transaction
)
encrypted_json_payload = facade.transaction_factory.attach_signature(
	encrypted_transaction, encrypted_signature
)
encrypted_transaction_hash = facade.hash_transaction(
	encrypted_transaction
)
print(f"Transaction hash: {encrypted_transaction_hash}")

encrypted_announce_request = urllib.request.Request(
	f"{NODE_URL}/transactions",
	data=encrypted_json_payload.encode("utf-8"),
	headers={"Content-Type": "application/json"},
	method="PUT",
)
with urllib.request.urlopen(encrypted_announce_request) as response:
	print(f"Encrypted message transaction announced\n")

# ===== RECEIVING ENCRYPTED MESSAGE =====
print("=== Receiving Encrypted Message ===")

# Wait for confirmation
encrypted_tx_data = wait_for_confirmation(
	encrypted_transaction_hash, "Encrypted message transaction"
)

# Decode encrypted message using recipient's private key
recipient_message_encoder = MessageEncoder(recipient_key_pair)
received_encrypted_message = bytes.fromhex(
	encrypted_tx_data["transaction"]["message"]
)

# Get sender's public key from the transaction
sender_public_key_from_tx = PublicKey(
	encrypted_tx_data["transaction"]["signerPublicKey"]
)

(is_decoded, decrypted_message) = recipient_message_encoder.try_decode(
	sender_public_key_from_tx, received_encrypted_message
)

if is_decoded:
	message_text = decrypted_message.decode("utf-8")
	print(f"Recipient decrypted message: {message_text}")
else:
	print(f"Recipient failed to decrypt message")

