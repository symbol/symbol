from binascii import hexlify

from symbolchain import sc

from ..CryptoTypes import Hash256, PublicKey
from ..RuleBasedTransactionFactory import RuleBasedTransactionFactory
from .IdGenerator import generate_mosaic_id, generate_namespace_id
from .Network import Address


class TransactionFactory:
	"""Factory for creating Symbol transactions."""

	def __init__(self, network, type_parsing_rules=None):
		"""Creates a factory for the specified network."""
		self.factory = self._build_rules(type_parsing_rules)
		self.network = network

	def _create_and_extend(self, transaction_descriptor, factory_class):
		transaction = self.factory.create_from_factory(factory_class.create_by_name, {
			**transaction_descriptor,
			'network': self.network.identifier
		})

		# autogenerate artifact ids
		if sc.TransactionType.NAMESPACE_REGISTRATION == transaction.type_:
			transaction.id = sc.NamespaceId(generate_namespace_id(transaction.name.decode('utf8'), transaction.parent_id.value))
		elif sc.TransactionType.MOSAIC_DEFINITION == transaction.type_:
			address = self.network.public_key_to_address(PublicKey(transaction.signer_public_key.bytes))
			transaction.id = sc.MosaicId(generate_mosaic_id(address, transaction.nonce.value))

		return transaction

	def create(self, transaction_descriptor):
		"""Creates a transaction from a transaction descriptor."""
		return self._create_and_extend(transaction_descriptor, sc.TransactionFactory)

	def create_embedded(self, transaction_descriptor):
		"""Creates an embedded transaction from a transaction descriptor."""
		return self._create_and_extend(transaction_descriptor, sc.EmbeddedTransactionFactory)

	@staticmethod
	def attach_signature(transaction, signature):
		"""Attaches a signature to a transaction."""
		transaction.signature = sc.Signature(signature.bytes)

		transaction_buffer = transaction.serialize()
		hex_payload = hexlify(transaction_buffer).decode('utf8').upper()
		json_payload = f'{{"payload": "{hex_payload}"}}'
		return json_payload.encode('utf8')

	@staticmethod
	def _symbol_type_converter(value):
		if isinstance(value, Address):
			return sc.UnresolvedAddress(value.bytes)

		return None

	@staticmethod
	def _build_rules(base_type_parsing_rules):
		factory = RuleBasedTransactionFactory(sc, TransactionFactory._symbol_type_converter, base_type_parsing_rules)
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
