from .ByteArray import ByteArray
from .CryptoTypes import Hash256, PublicKey, Signature
from .TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .TypeParserBuilder import build_type_hints_map


def basic_type_converter(module, value):
	if isinstance(value, PublicKey):
		return module.PublicKey(value.bytes)

	if isinstance(value, Hash256):
		return module.Hash256(value.bytes)

	if isinstance(value, Signature):
		return module.Signature(value.bytes)

	if isinstance(value, ByteArray):
		raise ValueError('ByteArray not allowed ')

	return value


class BasicTransactionFactory:
	"""Factory for creating transactions."""

	def __init__(self, transaction_network, type_converter, extended_type_parsing_rules):
		"""Creates a basic factory for the specified network."""
		self.transaction_network = transaction_network
		self.type_converter = type_converter
		self.extended_type_parsing_rules = extended_type_parsing_rules

	def _create(self, transaction_descriptor, factory_class):
		processor = TransactionDescriptorProcessor(transaction_descriptor, self.extended_type_parsing_rules, self.type_converter)

		processor.set_type_hints({'signer_public_key': PublicKey})

		transaction_type = processor.lookup_value('type')
		transaction = factory_class.create_by_name(transaction_type)
		transaction.signer_public_key = processor.lookup_value('signer_public_key')
		transaction.network = self.transaction_network

		all_type_hints = build_type_hints_map(transaction)
		processor.set_type_hints(all_type_hints)
		processor.copy_to(transaction, ['type', 'signer_public_key'])

		return transaction

	@staticmethod
	def _encode_string_field(obj, key):
		value = getattr(obj, key)
		if isinstance(value, str):
			setattr(obj, key, value.encode('utf8'))

	def _auto_encode_strings(self, transaction_descriptor, transaction):
		all_type_hints = build_type_hints_map(transaction)

		# auto encode "top-level" strings
		# _todo: maybe instead of doing this, there should be hint on: `namespaceRegistration.name` and `transfer.message`
		for key in transaction_descriptor.keys():
			if 'type' == key:
				key = 'type_'

			if key in all_type_hints:
				continue

			self._encode_string_field(transaction, key)
