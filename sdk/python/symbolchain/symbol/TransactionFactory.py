from binascii import hexlify

from symbolchain import sc

from ..CryptoTypes import Hash256, PublicKey
from ..RuleBasedTransactionFactory import RuleBasedTransactionFactory
from .IdGenerator import generate_mosaic_id, generate_namespace_id
from .Network import Address


class TransactionFactory:
	"""Factory for creating Symbol transactions."""

	def __init__(self, network, type_rule_overrides=None):
		"""Creates a factory for the specified network."""
		self.factory = self._build_rules(type_rule_overrides)
		self.network = network

	@staticmethod
	def lookup_transaction_name(transaction_type, transaction_version):
		"""Looks up the friendly name for the specified transaction."""
		return f'{str(transaction_type)[str(transaction_type).index(".") + 1:].lower()}_transaction_v{transaction_version}'

	def _create_and_extend(self, transaction_descriptor, autosort, factory_class):
		transaction = self.factory.create_from_factory(factory_class.create_by_name, {
			**transaction_descriptor,
			'network': self.network.identifier
		})
		if autosort:
			transaction.sort()

		# autogenerate artifact ids
		if sc.TransactionType.NAMESPACE_REGISTRATION == transaction.type_:
			parent_id = transaction.parent_id.value if sc.NamespaceRegistrationType.CHILD == transaction.registration_type else 0
			transaction.id = sc.NamespaceId(generate_namespace_id(transaction.name.decode('utf8'), parent_id))
		elif sc.TransactionType.MOSAIC_DEFINITION == transaction.type_:
			address = self.network.public_key_to_address(PublicKey(transaction.signer_public_key.bytes))
			transaction.id = sc.MosaicId(generate_mosaic_id(address, transaction.nonce.value))

		return transaction

	def create(self, transaction_descriptor, autosort=True):
		"""
		Creates a transaction from a transaction descriptor.
		When autosort is set (default), descriptor arrays requiring ordering will be automatically sorted.
		When unset, descriptor arrays will be presumed to be already sorted.
		"""
		return self._create_and_extend(transaction_descriptor, autosort, sc.TransactionFactory)

	def create_embedded(self, transaction_descriptor, autosort=True):
		"""
		Creates an embedded transaction from a transaction descriptor.
		When autosort is set (default), descriptor arrays requiring ordering will be automatically sorted.
		When unset, descriptor arrays will be presumed to be already sorted.
		"""
		return self._create_and_extend(transaction_descriptor, autosort, sc.EmbeddedTransactionFactory)

	@staticmethod
	def attach_signature(transaction, signature):
		"""Attaches a signature to a transaction."""
		transaction.signature = sc.Signature(signature.bytes)

		transaction_buffer = transaction.serialize()
		hex_payload = hexlify(transaction_buffer).decode('utf8').upper()
		json_payload = f'{{"payload": "{hex_payload}"}}'
		return json_payload

	@staticmethod
	def _symbol_type_converter(value):
		if isinstance(value, Address):
			return sc.UnresolvedAddress(value.bytes)

		return None

	@staticmethod
	def _build_rules(type_rule_overrides):
		factory = RuleBasedTransactionFactory(sc, TransactionFactory._symbol_type_converter, type_rule_overrides)
		factory.autodetect()

		factory.add_struct_parser('UnresolvedMosaic')

		sdk_type_mapping = {
			'UnresolvedAddress': Address,
			'Address': Address,
			'Hash256': Hash256,
			'PublicKey': PublicKey,
			'VotingPublicKey': PublicKey,
		}
		for name, typename in sdk_type_mapping.items():
			factory.add_pod_parser(name, typename)

		for name in ['UnresolvedMosaicId', 'TransactionType', 'UnresolvedAddress', 'struct:UnresolvedMosaic']:
			factory.add_array_parser(name)

		return factory
