import hashlib
from datetime import datetime, timezone

from .. import sc
from ..CryptoTypes import Hash256, PublicKey, Signature
from ..Network import NetworkLocator
from ..symbol.KeyPair import KeyPair, Verifier
from ..symbol.Merkle import MerkleHashBuilder
from ..symbol.MessageEncoder import MessageEncoder
from ..symbol.Network import Address, Network
from ..symbol.SharedKey import SharedKey
from ..symbol.TransactionFactory import TransactionFactory

TRANSACTION_HEADER_SIZE = sum(field[1] for field in [
	('size', 4),
	('reserved1', 4),
	('signature', Signature.SIZE),
	('signer', PublicKey.SIZE),
	('reserved2', 4)
])

PRE_V3_AGGREGATE_HASHED_SIZE = sum(field[1] for field in [
	('version_network_type', 4),
	('max_fee', 8),
	('deadline', 8),
	('transactions_hash', Hash256.SIZE)
])

AGGREGATE_HASHED_SIZE = PRE_V3_AGGREGATE_HASHED_SIZE + sum(field[1] for field in [
	('payload_size', 4)
])


# region SymbolPublicAccount / SymbolAccount

class SymbolPublicAccount:
	"""Symbol public account."""

	def __init__(self, facade, public_key):
		"""Creates a Symbol public account."""
		self._facade = facade
		self.public_key = public_key
		self.address = self._facade.network.public_key_to_address(self.public_key)


class SymbolAccount(SymbolPublicAccount):
	"""Symbol account."""

	def __init__(self, facade, key_pair):
		"""Creates a Symbol account."""
		super().__init__(facade, key_pair.public_key)
		self.key_pair = key_pair

	def message_encoder(self):
		"""Creates a message encoder that can be used for encrypting and encoding messages between two parties."""
		return MessageEncoder(self.key_pair)

	def sign_transaction(self, transaction):
		"""Signs a Symbol transaction."""
		return self._facade.sign_transaction(self.key_pair, transaction)

	def cosign_transaction(self, transaction, detached=False):
		"""Cosigns a Symbol transaction."""
		return self._facade.cosign_transaction(self.key_pair, transaction, detached)

	def cosign_transaction_hash(self, transaction_hash, detached=False):
		"""Cosigns a Symbol transaction hash."""
		return self._facade.cosign_transaction_hash(self.key_pair, transaction_hash, detached)

# endregion


class SymbolFacade:
	"""Facade used to interact with Symbol blockchain."""

	BIP32_CURVE_NAME = 'ed25519'

	Address = Address  # pylint: disable=duplicate-code
	KeyPair = KeyPair  # pylint: disable=duplicate-code
	Verifier = Verifier
	SharedKey = SharedKey

	def __init__(self, network, account_descriptor_repository=None):
		"""Creates a Symbol facade."""
		self.network = NetworkLocator.find_by_name(Network.NETWORKS, network) if isinstance(network, str) else network
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

	def now(self):
		"""Creates a network timestamp representing the current time."""
		return self.network.from_datetime(datetime.now(timezone.utc))

	def create_public_account(self, public_key):
		"""Creates a Symbol public account from a public key."""
		return SymbolPublicAccount(self, public_key)

	def create_account(self, private_key):
		"""Creates a Symbol account from a private key."""
		return SymbolAccount(self, KeyPair(private_key))

	def hash_transaction(self, transaction):
		"""Hashes a Symbol transaction."""
		hasher = hashlib.sha3_256()
		hasher.update(transaction.signature.bytes)
		hasher.update(transaction.signer_public_key.bytes)
		hasher.update(self.network.generation_hash_seed.bytes)
		hasher.update(self._transaction_data_buffer(transaction.serialize()))
		return Hash256(hasher.digest())

	def extract_signing_payload(self, transaction):
		"""Gets the payload to sign given a Symbol transaction."""
		sign_buffer = self.network.generation_hash_seed.bytes
		sign_buffer += self._transaction_data_buffer(transaction.serialize())
		return sign_buffer

	def sign_transaction(self, key_pair, transaction):
		"""Signs a Symbol transaction."""
		return key_pair.sign(self.extract_signing_payload(transaction))

	def verify_transaction(self, transaction, signature):
		"""Verifies a Symbol transaction."""
		return Verifier(transaction.signer_public_key).verify(self.extract_signing_payload(transaction), signature)

	@staticmethod
	def cosign_transaction_hash(key_pair, transaction_hash, detached=False):
		"""Cosigns a Symbol transaction hash."""
		cosignature = sc.DetachedCosignature() if detached else sc.Cosignature()
		if detached:
			cosignature.parent_hash = sc.Hash256(transaction_hash.bytes)

		cosignature.version = 0
		cosignature.signer_public_key = sc.PublicKey(key_pair.public_key.bytes)
		cosignature.signature = sc.Signature(key_pair.sign(transaction_hash.bytes).bytes)
		return cosignature

	def cosign_transaction(self, key_pair, transaction, detached=False):
		"""Cosigns a Symbol transaction."""
		transaction_hash = self.hash_transaction(transaction)
		return self.cosign_transaction_hash(key_pair, transaction_hash, detached)

	@staticmethod
	def hash_embedded_transactions(embedded_transactions):
		"""Hashes embedded transactions of an aggregate."""
		hash_builder = MerkleHashBuilder()
		for embedded_transaction in embedded_transactions:
			hash_builder.update(Hash256(hashlib.sha3_256(embedded_transaction.serialize()).digest()))

		return hash_builder.final()

	def bip32_path(self, account_id):
		"""Creates a network compatible BIP32 path for the specified account."""
		return [44, 4343 if 'mainnet' == self.network.name else 1, account_id, 0, 0]

	@staticmethod
	def bip32_node_to_key_pair(bip32_node):
		"""Derives a Symbol KeyPair from a BIP32 node."""
		return KeyPair(bip32_node.private_key)

	@staticmethod
	def _is_aggregate_transaction(transaction_buffer):
		transaction_type_offset = TRANSACTION_HEADER_SIZE + 2  # skip version and network byte
		transaction_type = (transaction_buffer[transaction_type_offset + 1] << 8) + transaction_buffer[transaction_type_offset]
		aggregate_types = [sc.TransactionType.AGGREGATE_BONDED.value, sc.TransactionType.AGGREGATE_COMPLETE.value]
		return transaction_type in aggregate_types

	@staticmethod
	def _transaction_data_buffer(transaction_buffer):
		data_buffer_start = TRANSACTION_HEADER_SIZE
		data_buffer_end = len(transaction_buffer)
		if SymbolFacade._is_aggregate_transaction(transaction_buffer):
			version = transaction_buffer[TRANSACTION_HEADER_SIZE]
			data_buffer_end = TRANSACTION_HEADER_SIZE + (AGGREGATE_HASHED_SIZE if version >= 3 else PRE_V3_AGGREGATE_HASHED_SIZE)

		return transaction_buffer[data_buffer_start:data_buffer_end]
