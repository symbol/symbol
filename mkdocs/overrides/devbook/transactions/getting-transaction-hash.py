import os
from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.KeyPair import KeyPair
from symbolchain.symbol.Network import NetworkTimestamp

# Initialize the facade for testnet
facade = SymbolFacade("testnet")

# Set up the signing key pair
private_key = os.environ.get(
	"PRIVATE_KEY",
	"0000000000000000000000000000000000000000000000000000000000000000",
)
key_pair = KeyPair(PrivateKey(private_key))

# Set up transaction parameters
current_time = NetworkTimestamp(1000000)  # Current network time
deadline = current_time.add_hours(2).timestamp
recipient_address = "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I"

# Create a transaction
transaction = facade.transaction_factory.create(
	{
		"type": "transfer_transaction_v1",
		"signer_public_key": key_pair.public_key,
		"deadline": deadline,
		"recipient_address": recipient_address,
		"mosaics": [{"mosaic_id": 0x6BED913FA20223F8, "amount": 1_000_000}],
	}
)

# Sign the transaction
signature = facade.sign_transaction(key_pair, transaction)
facade.transaction_factory.attach_signature(transaction, signature)

# Get the transaction hash
transaction_hash = facade.hash_transaction(transaction).bytes.hex().upper()

print(f"Transaction hash: {transaction_hash}")
