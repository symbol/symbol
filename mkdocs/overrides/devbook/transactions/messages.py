import json
import os
import time
import urllib.request
from binascii import hexlify

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.MessageEncoder import MessageEncoder
from symbolchain.symbol.Network import NetworkTimestamp
from symbolchain.symbol.IdGenerator import generate_mosaic_alias_id
from symbolchain.sc import Amount

# Configuration
NODE_URL = os.environ.get(
    "NODE_URL", "http://ngl-dual-101.testnet.symboldev.network:3000"
)

# Set up sender and recipient accounts
facade = SymbolFacade("testnet")
sender_private_key = os.environ.get(
    "PRIVATE_KEY",
    "0000000000000000000000000000000000000000000000000000000000000000",
)

sender_key_pair = facade.KeyPair(PrivateKey(sender_private_key))
sender_address = facade.network.public_key_to_address(
    sender_key_pair.public_key
)

# Set up recipient (using public key only)
recipient_public_key_string = os.environ.get(
    "RECIPIENT_PUBLIC_KEY",
    "D04AB232742BB4AB3A1368BD4615E4E6D0224AB71A016BAF8520A332C9778737",
)

recipient_public_key = PublicKey(recipient_public_key_string)
recipient_address = facade.network.public_key_to_address(recipient_public_key)
print(f"Sender address: {sender_address}")
print(f"Recipient address: {recipient_address}\n")

try:
    # ===== PLAIN TEXT MESSAGE =====
    print("=== Sending Plain Text Message ===")

    # Fetch current network time
    time_path = "/node/time"
    print(f"Fetching current network time from {time_path}")
    with urllib.request.urlopen(f"{NODE_URL}{time_path}") as response:
        response_json = json.loads(response.read().decode())
        timestamp = NetworkTimestamp(
            int(response_json["communicationTimestamps"]["receiveTimestamp"])
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
    transaction = facade.transaction_factory.create(
        {
            "type": "transfer_transaction_v1",
            "signer_public_key": sender_key_pair.public_key,
            "deadline": timestamp.add_hours(2).timestamp,
            "recipient_address": recipient_address,
            "mosaics": [
                {
                    "mosaic_id": generate_mosaic_alias_id("symbol.xym"),
                    "amount": 1_000_000,  # 1 XYM
                }
            ],
            "message": plain_message,
        }
    )
    transaction.fee = Amount(fee_mult * transaction.size)

    # Sign and announce the transaction
    signature = facade.sign_transaction(sender_key_pair, transaction)
    json_payload = facade.transaction_factory.attach_signature(
        transaction, signature
    )
    transaction_hash = facade.hash_transaction(transaction)
    print(f"Transaction hash: {transaction_hash}")

    announce_request = urllib.request.Request(
        f"{NODE_URL}/transactions",
        data=json_payload.encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="PUT",
    )
    with urllib.request.urlopen(announce_request) as response:
        print(f"Plain message transaction announced\n")

    # Wait a moment before sending the next transaction
    time.sleep(2)

    # ===== ENCRYPTED MESSAGE =====
    print("=== Sending Encrypted Message ===")

    # Fetch updated network time
    with urllib.request.urlopen(f"{NODE_URL}{time_path}") as response:
        response_json = json.loads(response.read().decode())
        timestamp = NetworkTimestamp(
            int(response_json["communicationTimestamps"]["receiveTimestamp"])
        )

    # Create a message encoder with sender's key pair
    message_encoder = MessageEncoder(sender_key_pair)

    # Encrypt the message using recipient's public key
    secret_message = "This is a secret message!".encode("utf-8")
    encrypted_payload = message_encoder.encode(
        recipient_public_key, secret_message
    )
    print(f"Original message: {secret_message.decode('utf-8')}")
    print(f"Encrypted payload: {hexlify(encrypted_payload).decode('utf-8')}")

    # Build transfer transaction with encrypted message
    transaction = facade.transaction_factory.create(
        {
            "type": "transfer_transaction_v1",
            "signer_public_key": sender_key_pair.public_key,
            "deadline": timestamp.add_hours(2).timestamp,
            "recipient_address": recipient_address,
            "mosaics": [
                {
                    "mosaic_id": generate_mosaic_alias_id("symbol.xym"),
                    "amount": 1_000_000,  # 1 XYM
                }
            ],
            "message": encrypted_payload,
        }
    )
    transaction.fee = Amount(fee_mult * transaction.size)

    # Sign and announce the transaction
    signature = facade.sign_transaction(sender_key_pair, transaction)
    json_payload = facade.transaction_factory.attach_signature(
        transaction, signature
    )
    transaction_hash = facade.hash_transaction(transaction)
    print(f"Transaction hash: {transaction_hash}")

    announce_request = urllib.request.Request(
        f"{NODE_URL}/transactions",
        data=json_payload.encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="PUT",
    )
    with urllib.request.urlopen(announce_request) as response:
        print(f"Encrypted message transaction announced\n")

    # ===== DECRYPTING MESSAGE =====
    print("=== Decrypting Message ===")

    # Sender can decrypt using recipient's public key
    (is_decoded, decrypted_message) = message_encoder.try_decode(
        recipient_public_key, encrypted_payload
    )
    if is_decoded:
        message_text = decrypted_message.decode("utf-8")
        print(f"Sender verified encrypted message: {message_text}")
    else:
        print(f"Sender failed to decrypt message")

    # The recipient would decrypt using their private key:
    # recipient_encoder = MessageEncoder(recipient_key_pair)
    # (is_decoded, msg) = recipient_encoder.try_decode(
    #   sender_key_pair.public_key, encrypted_payload
    # )

    print("\nBoth transactions have been sent successfully!")

except urllib.error.URLError as e:
    print(e.reason)
