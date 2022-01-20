import sha3

from ..CryptoTypes import Hash256, PublicKey, Signature
from ..Network import NetworkLocator
from ..sc import TransactionType
from ..symbol.KeyPair import KeyPair, Verifier
from ..symbol.Network import Address, Network
from ..symbol.TransactionFactory import TransactionFactory

TRANSACTION_HEADER_SIZE = sum(field[1] for field in [
	('size', 4),
	('reserved1', 4),
	('signature', Signature.SIZE),
	('signer', PublicKey.SIZE),
	('reserved2', 4)
])

AGGREGATE_HASHED_SIZE = sum(field[1] for field in [
	('version_network_type', 4),
	('max_fee', 8),
	('deadline', 8),
	('transactions_hash', Hash256.SIZE)
])


class SymbolFacade:
	"""Facade used to interact with symbol blockchain."""

	BIP32_COIN_ID = 4343
	BIP32_CURVE_NAME = 'ed25519'

	Address = Address
	KeyPair = KeyPair
	Verifier = Verifier

	def __init__(self, symbol_network_name, account_descriptor_repository=None):
		"""Creates a symbol facade."""
		self.network = NetworkLocator.find_by_name(Network.NETWORKS, symbol_network_name)
		self.account_descriptor_repository = account_descriptor_repository
		self.transaction_factory = self._create_symbol_transaction_factory()

	def _create_symbol_transaction_factory(self):
		type_parsing_rules = None
		if self.account_descriptor_repository:
			type_to_property_mapping = self._create_symbol_type_to_property_mapping()
			type_parsing_rules = self.account_descriptor_repository.to_type_parsing_rules_map(type_to_property_mapping)

		return TransactionFactory(self.network, type_parsing_rules)

	# NOTE: currently `TypeParserBuilder.create_sdk_wrapper`` assumes SDK types are used as keys (although could be `sc` types as well)
	# resulting sdk type will be converted further via `symbol_type_converter`
	@staticmethod
	def _create_symbol_type_to_property_mapping():
		return {
			Address: 'address',
			PublicKey: 'public_key',
		}

	def hash_transaction(self, transaction):
		"""Hashes a symbol transaction."""
		hasher = sha3.sha3_256()
		hasher.update(transaction.signature.bytes)
		hasher.update(transaction.signer_public_key.bytes)
		hasher.update(self.network.generation_hash_seed.bytes)
		hasher.update(self._transaction_data_buffer(transaction.serialize()))
		return Hash256(hasher.digest())

	def sign_transaction(self, key_pair, transaction):
		"""Signs a symbol transaction."""
		sign_buffer = self.network.generation_hash_seed.bytes
		sign_buffer += self._transaction_data_buffer(transaction.serialize())
		return key_pair.sign(sign_buffer)

	def verify_transaction(self, transaction, signature):
		"""Verifies a symbol transaction."""
		verify_buffer = self.network.generation_hash_seed.bytes
		verify_buffer += self._transaction_data_buffer(transaction.serialize())
		return Verifier(PublicKey(transaction.signer_public_key.bytes)).verify(verify_buffer, signature)

	@staticmethod
	def bip32_node_to_key_pair(bip32_node):
		"""Derives a symbol KeyPair from a BIP32 node."""
		return KeyPair(bip32_node.private_key)

	@staticmethod
	def _transaction_data_buffer(transaction_buffer):
		data_buffer_start = TRANSACTION_HEADER_SIZE
		data_buffer_end = len(transaction_buffer)
		if SymbolFacade._is_aggregate_transaction(transaction_buffer):
			data_buffer_end = TRANSACTION_HEADER_SIZE + AGGREGATE_HASHED_SIZE

		return transaction_buffer[data_buffer_start:data_buffer_end]

	@staticmethod
	def _is_aggregate_transaction(transaction_buffer):
		transaction_type_offset = TRANSACTION_HEADER_SIZE + 2  # skip version and network byte
		transaction_type = (transaction_buffer[transaction_type_offset + 1] << 8) + transaction_buffer[transaction_type_offset]
		aggregate_types = [TransactionType.AGGREGATE_BONDED.value, TransactionType.AGGREGATE_COMPLETE.value]
		return transaction_type in aggregate_types
