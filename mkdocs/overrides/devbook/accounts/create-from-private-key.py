import os
from symbolchain.CryptoTypes import PrivateKey
from symbolchain.facade.SymbolFacade import SymbolFacade

# Initialize the facade for the testnet network
facade = SymbolFacade('testnet')

# ===== RANDOM ACCOUNT =====
print('Creating a random account...')

# Generate a random private key
random_private_key = PrivateKey.random()

# Create a key pair from the private key
random_key_pair = facade.KeyPair(random_private_key)

# Derive the address from the public key
random_address = facade.network.public_key_to_address(
	random_key_pair.public_key
)

# Output the account details
print(f'    Address: {random_address}')
print(f'    Public key: {random_key_pair.public_key}')
print(f'    Private key: {random_key_pair.private_key}')

# ===== ACCOUNT FROM PRIVATE KEY =====
print('\nCreating an account from a private key...')

# Load an existing private key from environment variable or use default
existing_private_key_string = os.environ.get('PRIVATE_KEY',
	'0000000000000000000000000000000000000000000000000000000000000000')
existing_private_key = PrivateKey(existing_private_key_string)

# Create a key pair from the private key
existing_key_pair = facade.KeyPair(existing_private_key)

# Derive the address from the public key
existing_address = facade.network.public_key_to_address(
	existing_key_pair.public_key
)

# Output the account details
print(f'    Address: {existing_address}')
print(f'    Public key: {existing_key_pair.public_key}')
print(f'    Private key: {existing_key_pair.private_key}')