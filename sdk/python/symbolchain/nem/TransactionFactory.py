from binascii import hexlify

from symbolchain import nc

from ..BasicTransactionFactory import BasicTransactionFactory, basic_type_converter
from .ExtendedTypeParsingRules import extend_type_parsing_rules
from .Network import Address


def nem_type_converter(value):
	if isinstance(value, Address):
		# yes, unfortunatelly, nem's Address is 40 bytes string, but we need to pass it as actual bytes not to confuse ByteArray
		return nc.Address(str(value).encode('utf8'))

	return basic_type_converter(nc, value)


class TransactionFactory(BasicTransactionFactory):
	"""Factory for creating NEM transactions."""

	def __init__(self, network, type_parsing_rules=None):
		"""Creates a factory for the specified network."""
		super().__init__(
			nc.NetworkType(network.identifier),
			nem_type_converter,
			extend_type_parsing_rules(nem_type_converter, type_parsing_rules))

	def _create_and_extend(self, transaction_descriptor):
		transaction = self._create(transaction_descriptor, nc.TransactionFactory)
		self._auto_encode_strings(transaction_descriptor, transaction)

		# hack: explicitely translate transfer message
		if nc.TransactionType.TRANSFER == transaction.type_:
			self._encode_string_field(transaction.message, 'message')

		return transaction

	def create(self, transaction_descriptor):
		"""Creates a transaction from a transaction descriptor."""
		return self._create_and_extend(transaction_descriptor)

	@staticmethod
	def to_non_verifiable_transaction(transaction):
		"""Converts transaction to non_verifiable transaction"""
		class_name = type(transaction).__name__
		if not class_name.startswith('NonVerifiable'):
			class_name = 'NonVerifiable' + class_name

		non_verifiable_class = getattr(nc, class_name)
		non_verifiable_transaction = non_verifiable_class()
		for key in dir(non_verifiable_transaction):
			# isupper() to quickly filter out class properties like TRANSACTION_VERSION or TYPE_HINTS
			if key.startswith('_') or key[0].isupper() or key in ['size', 'serialize', 'deserialize']:
				continue

			setattr(non_verifiable_transaction, key, getattr(transaction, key))

		return non_verifiable_transaction

	@staticmethod
	def attach_signature(transaction, signature):
		"""Attaches a signature to a transaction."""
		transaction.signature = nc.Signature(signature.bytes)

		transaction_buffer = TransactionFactory.to_non_verifiable_transaction(transaction).serialize()
		transaction_buffer_hex = hexlify(transaction_buffer).decode('utf8').upper()
		signature_hex = hexlify(signature.bytes).decode('utf8').upper()
		json_payload = f'{{"data":"{transaction_buffer_hex}", "signature":"{signature_hex}"}}'
		return json_payload.encode('utf8')
