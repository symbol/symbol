from binascii import hexlify

from symbolchain import nc

from ..CryptoTypes import Hash256, PublicKey
from ..RuleBasedTransactionFactory import RuleBasedTransactionFactory
from .Network import Address


class TransactionFactory:
	"""Factory for creating NEM transactions."""

	def __init__(self, network, type_parsing_rules=None):
		"""Creates a factory for the specified network."""
		self.factory = self._build_rules(type_parsing_rules)
		self.network = network

	def create(self, transaction_descriptor):
		"""Creates a transaction from a transaction descriptor."""
		transaction = self.factory.create_from_factory(nc.TransactionFactory.create_by_name, {
			**transaction_descriptor,
			'network': self.network.identifier
		})

		# hack: explicitly translate transfer message
		if nc.TransactionType.TRANSFER == transaction.type_ and isinstance(transaction.message.message, str):
			transaction.message.message = transaction.message.message.encode('utf8')

		return transaction

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

	@staticmethod
	def _nem_type_converter(value):
		if isinstance(value, Address):
			# yes, unfortunatelly, nem's Address is 40 bytes string, but we need to pass it as actual bytes not to confuse ByteArray
			return nc.Address(str(value).encode('utf8'))

		return None

	@staticmethod
	def _build_rules(base_type_parsing_rules):
		factory = RuleBasedTransactionFactory(nc, TransactionFactory._nem_type_converter, base_type_parsing_rules)
		factory.autodetect()

		struct_names = [
			'Message', 'NamespaceId', 'MosaicId', 'Mosaic', 'SizePrefixedMosaic', 'MosaicLevy',
			'MosaicProperty', 'SizePrefixedMosaicProperty', 'MosaicDefinition',
			'MultisigAccountModification', 'SizePrefixedMultisigAccountModification'
		]
		for name in struct_names:
			factory.add_struct_parser(name)

		sdk_type_mapping = {
			'Address': Address,
			'Hash256': Hash256,
			'PublicKey': PublicKey,
		}
		for name, typename in sdk_type_mapping.items():
			factory.add_pod_parser(name, typename)

		for name in ['struct:SizePrefixedMosaic', 'struct:SizePrefixedMosaicProperty', 'struct:SizePrefixedMultisigAccountModification']:
			factory.add_array_parser(name)

		return factory
