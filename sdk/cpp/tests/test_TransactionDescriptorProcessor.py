import unittest
from types import SimpleNamespace

from symbolchain.CryptoTypes import PublicKey
from symbolchain.TransactionDescriptorProcessor import TransactionDescriptorProcessor


class TransactionDescriptorProcessorTest(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	# region test utils

	@staticmethod
	def _create_processor(extended_descriptor=None):
		transaction_descriptor = {
			'type': 'transfer',
			'timestamp': 12345,
			'signer': 'signer_name',
			'recipient': 'recipient_name',
			'message': 'hello world'
		}
		transaction_descriptor.update(extended_descriptor or {})
		type_parsing_rules = {PublicKey: lambda name: f'{name} PUBLICKEY'}
		processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules)
		processor.set_type_hints({'signer': PublicKey, 'timestamp': int})
		return processor

	@staticmethod
	def _create_processor_with_converter(deadline_value=300):
		transaction_descriptor = {
			'type': 'transfer',
			'timestamp': 12345,
			'signer': 'signer_name',
			'recipient': 'recipient_name',
			'message': 'hello world',
			'fee': 100,
			'deadline': deadline_value
		}
		type_parsing_rules = {int: lambda value: value + 42}

		def type_converter(value):
			return value * 2 if isinstance(value, int) else value

		processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules, type_converter)
		processor.set_type_hints({'timestamp': int})
		return processor

	# endregion

	# region lookup_value

	def _assert_cannot_lookup_value_when_descriptor_does_not_contain_key(self, processor_factory):
		# Arrange:
		processor = processor_factory()

		# Act + Assert:
		with self.assertRaises(ValueError):
			processor.lookup_value('foo')

	def test_cannot_lookup_value_when_descriptor_does_not_contain_key(self):
		self._assert_cannot_lookup_value_when_descriptor_does_not_contain_key(self._create_processor)

	def test_cannot_lookup_value_when_descriptor_does_not_contain_key_converter(self):
		self._assert_cannot_lookup_value_when_descriptor_does_not_contain_key(self._create_processor_with_converter)

	def _assert_can_lookup_value_message(self, processor_factory):
		# Arrange:
		processor = processor_factory()

		# Act:
		message = processor.lookup_value('message')

		# Assert:
		self.assertEqual('hello world', message)

	def test_can_lookup_value_without_type_hint(self):
		self._assert_can_lookup_value_message(self._create_processor)

	def test_can_lookup_value_without_conversion(self):
		self._assert_can_lookup_value_message(self._create_processor_with_converter)

	def test_can_lookup_value_with_type_hint_but_without_custom_rule(self):
		# Arrange:
		processor = self._create_processor()

		# Act:
		timestamp = processor.lookup_value('timestamp')

		# Assert:
		self.assertEqual(12345, timestamp)

	def test_can_lookup_value_when_hints_are_applied_before_conversion(self):
		# Arrange:
		processor = self._create_processor_with_converter()

		# Act:
		timestamp = processor.lookup_value('timestamp')

		# Assert: (12345 + 42) * 2
		self.assertEqual(24774, timestamp)

	def test_can_lookup_value_with_type_hint_and_with_custom_rule(self):
		# Arrange:
		processor = self._create_processor()

		# Act:
		signer = processor.lookup_value('signer')

		# Assert:
		self.assertEqual('signer_name PUBLICKEY', signer)

	def test_can_lookup_value_when_applying_converter_to_all_fields(self):
		# Arrange:
		processor = self._create_processor_with_converter()

		# Act:
		fee = processor.lookup_value('fee')
		deadline = processor.lookup_value('deadline')

		# Assert:
		self.assertEqual(200, fee)
		self.assertEqual(600, deadline)

	def test_can_lookup_value_when_applying_converter_to_all_array_elements(self):
		# Arrange: specify the deadline value as an array
		processor = self._create_processor_with_converter([100, 300, 600])

		# Act:
		deadline = processor.lookup_value('deadline')

		# Assert:
		self.assertEqual([200, 600, 1200], deadline)

	def test_can_lookup_value_with_zero_value(self):
		# Arrange: specify the deadline value as zero
		processor = self._create_processor_with_converter(0)

		# Act:
		deadline = processor.lookup_value('deadline')

		# Assert:
		self.assertEqual(0, deadline)

	def test_can_lookup_value_when_value_is_list(self):
		# Arrange:
		key = 'flags'
		value = [1, 2, 3]
		processor = self._create_processor({key: value})

		# Act:
		result = processor.lookup_value(key)

		# Assert:
		self.assertEqual(value, result)

	def test_can_lookup_value_when_value_is_iterable_and_not_list(self):
		# Arrange:
		key = 'flags'
		value = (4, 5, 6)
		processor = self._create_processor({key: value})

		# Act:
		result = processor.lookup_value(key)

		# Assert:
		self.assertEqual(value, result)  # Confirming no change in non-list iterable

	def test_can_lookup_value_when_value_is_not_iterable(self):
		# Arrange:
		key = 'flags'
		value = 'supply_mutable restrictable transferable revokable'
		processor = self._create_processor({key: value})

		# Act:
		result = processor.lookup_value(key)

		# Assert:
		self.assertEqual(value, result)  # Confirming no change for non-iterable value

	# endregion

	# region copy_to

	def test_cannot_copy_to_when_descriptor_contains_fields_not_in_transaction(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None)  # missing message

		# Act + Assert:
		with self.assertRaises(ValueError):
			processor.copy_to(transaction)

	def test_cannot_copy_to_when_descriptor_contains_computed_field(self):
		# Arrange:
		processor = self._create_processor({'message_envelope_size_computed': 123})
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None)

		# Act + Assert:
		with self.assertRaises(ValueError):
			processor.copy_to(transaction)

	def test_can_copy_to_when_transaction_contains_exact_fields_in_descriptor(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None)

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual(12345, transaction.timestamp)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual('recipient_name', transaction.recipient)
		self.assertEqual('hello world', transaction.message)

	def test_can_copy_to_when_transaction_contains_fields_not_in_descriptor(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None, foo=None)

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual(12345, transaction.timestamp)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual('recipient_name', transaction.recipient)
		self.assertEqual('hello world', transaction.message)
		self.assertEqual(None, transaction.foo)

	def test_can_copy_to_when_ignore_keys_is_not_empty(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None)

		# Act:
		processor.copy_to(transaction, ['type', 'recipient'])

		# Assert:
		self.assertEqual(None, transaction.type)
		self.assertEqual(12345, transaction.timestamp)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual(None, transaction.recipient)
		self.assertEqual('hello world', transaction.message)

	def test_can_copy_to_when_transaction_contains_iterable_attribute(self):
		# Arrange:
		transaction_descriptor = {
			'type': 'transfer',
			'signer': 'signer_name',
			'mosaics': [(1, 2), (3, 5)]
		}
		type_parsing_rules = {PublicKey: lambda name: f'{name} PUBLICKEY'}
		processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules)
		processor.set_type_hints({'signer': PublicKey})

		transaction = SimpleNamespace(type=None, signer=None, mosaics=[])

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual([(1, 2), (3, 5)], transaction.mosaics)

	def test_can_copy_to_when_transaction_contains_tuple_attribute(self):
		# Arrange:
		transaction_descriptor = {
			'type': 'transfer',
			'signer': 'signer_name',
			'payload': (1, 2)
		}
		type_parsing_rules = {PublicKey: lambda name: f'{name} PUBLICKEY'}
		processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules)
		processor.set_type_hints({'signer': PublicKey})

		transaction = SimpleNamespace(type=None, signer=None, payload=None)

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual((1, 2), transaction.payload)

	def test_can_copy_to_when_transaction_contains_bytes_attribute(self):
		# Arrange:
		transaction_descriptor = {
			'type': 'transfer',
			'signer': 'signer_name',
			'payload': b'FF554433'
		}
		type_parsing_rules = {PublicKey: lambda name: f'{name} PUBLICKEY'}
		processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules)
		processor.set_type_hints({'signer': PublicKey})

		transaction = SimpleNamespace(type=None, signer=None, payload=None)

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer)
		self.assertEqual(b'FF554433', transaction.payload)

	# endregion

	# region converter + copy_to

	def test_can_copy_to_with_custom_converter(self):
		# Arrange:
		processor = self._create_processor_with_converter()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None, fee=None, deadline=None)

		# Act:
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual(24774, transaction.timestamp)
		self.assertEqual('signer_name', transaction.signer)
		self.assertEqual('recipient_name', transaction.recipient)
		self.assertEqual('hello world', transaction.message)
		self.assertEqual(200, transaction.fee)
		self.assertEqual(600, transaction.deadline)

	# endregion

	# region set_type_hints + copy_to

	def test_can_change_type_hints(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None)

		# Act:
		processor.set_type_hints({'recipient': PublicKey})
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual(12345, transaction.timestamp)
		self.assertEqual('signer_name', transaction.signer)
		self.assertEqual('recipient_name PUBLICKEY', transaction.recipient)
		self.assertEqual('hello world', transaction.message)

	def test_can_clear_type_hints(self):
		# Arrange:
		processor = self._create_processor()
		transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None, message=None)

		# Act:
		processor.set_type_hints(None)
		processor.copy_to(transaction)

		# Assert:
		self.assertEqual('transfer', transaction.type)
		self.assertEqual(12345, transaction.timestamp)
		self.assertEqual('signer_name', transaction.signer)
		self.assertEqual('recipient_name', transaction.recipient)
		self.assertEqual('hello world', transaction.message)

	# endregion
