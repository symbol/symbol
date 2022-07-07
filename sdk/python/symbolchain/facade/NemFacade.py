import sha3

from ..CryptoTypes import Hash256, PrivateKey, PublicKey
from ..nem.KeyPair import KeyPair, Verifier
from ..nem.Network import Address, Network
from ..nem.SharedKey import SharedKey
from ..nem.TransactionFactory import TransactionFactory
from ..Network import NetworkLocator


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

	@staticmethod
	def hash_transaction(transaction):
		"""Hashes a NEM transaction."""
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)
		return Hash256(sha3.keccak_256(non_verifiable_transaction.serialize()).digest())

	@staticmethod
	def sign_transaction(key_pair, transaction):
		"""Signs a NEM transaction."""
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)
		return key_pair.sign(non_verifiable_transaction.serialize())

	@staticmethod
	def verify_transaction(transaction, signature):
		"""Verifies a NEM transaction."""
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)
		return Verifier(transaction.signer_public_key).verify(non_verifiable_transaction.serialize(), signature)

	def bip32_path(self, account_id):
		"""Creates a network compatible BIP32 path for the specified account."""
		return [44, 43 if 'mainnet' == self.network.name else 1, account_id, 0, 0]

	@staticmethod
	def bip32_node_to_key_pair(bip32_node):
		"""Derives a NEM KeyPair from a BIP32 node."""
		# BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
		return KeyPair(PrivateKey(bip32_node.private_key.bytes[::-1]))
