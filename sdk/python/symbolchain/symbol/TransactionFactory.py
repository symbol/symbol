from binascii import hexlify

from symbolchain import sc

from ..BasicTransactionFactory import BasicTransactionFactory, basic_type_converter
from ..CryptoTypes import PublicKey
from .ExtendedTypeParsingRules import extend_type_parsing_rules
from .IdGenerator import generate_mosaic_id, generate_namespace_id
from .Network import Address


def symbol_type_converter(value):
	if isinstance(value, Address):
		return sc.UnresolvedAddress(value.bytes)

	return basic_type_converter(sc, value)


class TransactionFactory(BasicTransactionFactory):
	"""Factory for creating Symbol transactions."""

	def __init__(self, network, type_parsing_rules=None):
		"""Creates a factory for the specified network."""
		super().__init__(
			sc.NetworkType(network.identifier),
			symbol_type_converter,
			extend_type_parsing_rules(symbol_type_converter, type_parsing_rules))
		self.network = network

	def _create_and_extend(self, transaction_descriptor, factory_class):
		transaction = self._create(transaction_descriptor, factory_class)

		# autogenerate artifact ids
		if sc.TransactionType.NAMESPACE_REGISTRATION == transaction.type_:
			transaction.id = sc.NamespaceId(generate_namespace_id(transaction.name, transaction.parent_id.value))
		elif sc.TransactionType.MOSAIC_DEFINITION == transaction.type_:
			address = self.network.public_key_to_address(PublicKey(transaction.signer_public_key.bytes))
			transaction.id = sc.MosaicId(generate_mosaic_id(address, transaction.nonce.value))

		self._auto_encode_strings(transaction_descriptor, transaction)
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
