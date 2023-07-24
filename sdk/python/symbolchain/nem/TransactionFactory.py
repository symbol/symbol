from binascii import hexlify

from symbolchain import nc

from ..CryptoTypes import Hash256, PublicKey
from ..RuleBasedTransactionFactory import RuleBasedTransactionFactory
from .Network import Address


class TransactionFactory:
	"""Factory for creating NEM transactions."""

	def __init__(self, network, type_rule_overrides=None):
		"""Creates a factory for the specified network."""
		self.factory = self._build_rules(type_rule_overrides)
		self.network = network

	@staticmethod
	def lookup_transaction_name(transaction_type, transaction_version):
		"""Looks up the friendly name for the specified transaction."""
		return f'{str(transaction_type)[str(transaction_type).index(".") + 1:].lower()}_transaction_v{transaction_version}'

	def create(self, transaction_descriptor, autosort=True):
		"""
		Creates a transaction from a transaction descriptor.
		When autosort is set (default), descriptor arrays requiring ordering will be automatically sorted.
		When unset, descriptor arrays will be presumed to be already sorted.
		"""
		transaction = self.factory.create_from_factory(nc.TransactionFactory.create_by_name, {
			**transaction_descriptor,
			'network': self.network.identifier
		})
		if autosort:
			transaction.sort()

		# hack: explicitly translate transfer message
		if nc.TransactionType.TRANSFER == transaction.type_ and transaction.message and isinstance(transaction.message.message, str):
			transaction.message.message = transaction.message.message.encode('utf8')

		return transaction

	@staticmethod
	def to_non_verifiable_transaction(transaction):
		"""Converts a transaction to a non-verifiable transaction."""
		non_verifiable_class_name = type(transaction).__name__
		if not non_verifiable_class_name.startswith('NonVerifiable'):
			non_verifiable_class_name = f'NonVerifiable{non_verifiable_class_name}'

		non_verifiable_class = getattr(nc, non_verifiable_class_name)
		non_verifiable_transaction = non_verifiable_class()
		for key in dir(non_verifiable_transaction):
			# isupper() to quickly filter out class properties like TRANSACTION_VERSION or TYPE_HINTS
			if key.startswith('_') or key[0].isupper() or key in ('size', 'serialize', 'deserialize') or key.endswith('_computed'):
				continue

			setattr(non_verifiable_transaction, key, getattr(transaction, key))

		return non_verifiable_transaction

	@staticmethod
	def attach_signature(transaction, signature):
		"""Attaches a signature to a transaction."""
		transaction.signature = nc.Signature(signature.bytes)

		transaction_hex = hexlify(TransactionFactory.to_non_verifiable_transaction(transaction).serialize()).decode('utf8').upper()
		signature_hex = str(signature)
		json_payload = f'{{"data":"{transaction_hex}", "signature":"{signature_hex}"}}'
		return json_payload

	@staticmethod
	def _nem_type_converter(value):
		if isinstance(value, Address):
			# yes, unfortunately, nem's Address is 40 bytes string, but we need to pass it as actual bytes not to confuse ByteArray
			return nc.Address(str(value).encode('utf8'))

		return None

	@staticmethod
	def _build_rules(type_rule_overrides):
		factory = RuleBasedTransactionFactory(nc, TransactionFactory._nem_type_converter, type_rule_overrides)
		factory.autodetect()

		struct_names = [
			'CosignatureV1', 'Message', 'NamespaceId', 'MosaicId', 'Mosaic', 'SizePrefixedMosaic', 'MosaicLevy',
			'MosaicProperty', 'SizePrefixedMosaicProperty', 'MosaicDefinition',
			'MultisigAccountModification', 'SizePrefixedMultisigAccountModification', 'SizePrefixedCosignatureV1'
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

		array_names = [
			'SizePrefixedMosaic', 'SizePrefixedMosaicProperty', 'SizePrefixedMultisigAccountModification', 'SizePrefixedCosignatureV1'
		]
		for name in array_names:
			factory.add_array_parser(f'struct:{name}')

		return factory
