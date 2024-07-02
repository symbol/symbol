from datetime import datetime, timezone

import sha3

from ..CryptoTypes import Hash256, PrivateKey, PublicKey
from ..nem.KeyPair import KeyPair, Verifier
from ..nem.MessageEncoder import MessageEncoder
from ..nem.Network import Address, Network
from ..nem.SharedKey import SharedKey
from ..nem.TransactionFactory import TransactionFactory
from ..Network import NetworkLocator

# region NemPublicAccount / NemAccount


class NemPublicAccount:
	"""NEM public account."""

	def __init__(self, facade, public_key):
		"""Creates a NEM public account."""
		self._facade = facade
		self.public_key = public_key
		self.address = self._facade.network.public_key_to_address(self.public_key)


class NemAccount(NemPublicAccount):
	"""NEM account."""

	def __init__(self, facade, key_pair):
		"""Creates a NEM account."""
		super().__init__(facade, key_pair.public_key)
		self.key_pair = key_pair

	def message_encoder(self):
		"""Creates a message encoder that can be used for encrypting and encoding messages between two parties."""
		return MessageEncoder(self.key_pair)

	def sign_transaction(self, transaction):
		"""Signs a NEM transaction."""
		return self._facade.sign_transaction(self.key_pair, transaction)

# endregion


class NemFacade:
	"""Facade used to interact with NEM blockchain."""

	BIP32_CURVE_NAME = 'ed25519-keccak'

	Address = Address  # pylint: disable=duplicate-code
	KeyPair = KeyPair  # pylint: disable=duplicate-code
	Verifier = Verifier
	SharedKey = SharedKey

	def __init__(self, network, account_descriptor_repository=None):
		"""Creates a NEM facade."""
		self.network = NetworkLocator.find_by_name(Network.NETWORKS, network) if isinstance(network, str) else network
		self.account_descriptor_repository = account_descriptor_repository
		self.transaction_factory = self._create_nem_transaction_factory()

	def _create_nem_transaction_factory(self):
		type_parsing_rules = None
		if self.account_descriptor_repository:
			type_parsing_rules = self.account_descriptor_repository.to_type_parsing_rules_map(self._create_nem_type_to_property_mapping())

		return TransactionFactory(self.network, type_parsing_rules)

	# NOTE: currently `TypeParserBuilder.create_sdk_wrapper`` assumes SDK types are used as keys (although could be `nc` types as well)
	# resulting sdk type will be converted further via `nem_type_converter`
	@staticmethod
	def _create_nem_type_to_property_mapping():
		return {
			Address: 'address',
			PublicKey: 'public_key'
		}

	def now(self):
		"""Creates a network timestamp representing the current time."""
		return self.network.from_datetime(datetime.now(timezone.utc))

	def create_public_account(self, public_key):
		"""Creates a NEM public account from a public key."""
		return NemPublicAccount(self, public_key)

	def create_account(self, private_key):
		"""Creates a NEM account from a private key."""
		return NemAccount(self, KeyPair(private_key))

	@staticmethod
	def hash_transaction(transaction):
		"""Hashes a NEM transaction."""
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)
		return Hash256(sha3.keccak_256(non_verifiable_transaction.serialize()).digest())

	@staticmethod
	def extract_signing_payload(transaction):
		"""Gets the payload to sign given a NEM transaction."""
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)
		return non_verifiable_transaction.serialize()

	def sign_transaction(self, key_pair, transaction):
		"""Signs a NEM transaction."""
		return key_pair.sign(self.extract_signing_payload(transaction))

	def verify_transaction(self, transaction, signature):
		"""Verifies a NEM transaction."""
		return Verifier(transaction.signer_public_key).verify(self.extract_signing_payload(transaction), signature)

	def bip32_path(self, account_id):
		"""Creates a network compatible BIP32 path for the specified account."""
		return [44, 43 if 'mainnet' == self.network.name else 1, account_id, 0, 0]

	@staticmethod
	def bip32_node_to_key_pair(bip32_node):
		"""Derives a NEM KeyPair from a BIP32 node."""
		# BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
		return KeyPair(PrivateKey(bip32_node.private_key.bytes[::-1]))
