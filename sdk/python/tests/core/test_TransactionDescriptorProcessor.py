import unittest
from types import SimpleNamespace

from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.TransactionDescriptorProcessor import TransactionDescriptorProcessor


class TransactionDescriptorProcessorTest(unittest.TestCase):
    # region lookup_value

    @staticmethod
    def _create_processor():
        transaction_descriptor = {
            'type': 'transfer',
            'timestamp': 12345,
            'signer': 'signer_name',
            'recipient': 'recipient_name',
            'message': 'hello world',
        }
        type_parsing_rules = {PublicKey: lambda name: name + ' PUBLICKEY'}
        processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules)
        processor.set_type_hints({'signer': PublicKey, 'timestamp': int})
        return processor

    @staticmethod
    def _type_converter(value):
        return value * 2 if isinstance(value, int) else value

    @staticmethod
    def _create_processor_with_converter():
        transaction_descriptor = {
            'type': 'transfer',
            'timestamp': 12345,
            'signer': 'signer_name',
            'recipient': 'recipient_name',
            'message': 'hello world',
            'fee': 100,
            'deadline': 300
        }
        type_parsing_rules = {int: lambda value: value + 42}
        type_converter = TransactionDescriptorProcessorTest._type_converter
        processor = TransactionDescriptorProcessor(transaction_descriptor, type_parsing_rules, type_converter)
        processor.set_type_hints({'timestamp': int})
        return processor

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

    def test_lookup_value_hints_are_applied_before_conversion(self):
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

    def test_lookup_value_applies_converter_to_all_fields(self):
        # Arrange:
        processor = self._create_processor_with_converter()

        # Act:
        fee = processor.lookup_value('fee')
        deadline = processor.lookup_value('deadline')

        # Assert:
        self.assertEqual(200, fee)
        self.assertEqual(600, deadline)

    # endregion

    # region copy_to

    def test_cannot_copy_to_when_descriptor_contains_fields_not_in_transaction(self):
        # Arrange:
        processor = self._create_processor()
        transaction = SimpleNamespace(type=None, timestamp=None, signer=None, recipient=None)  # missing message

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
        type_parsing_rules = {PublicKey: lambda name: name + ' PUBLICKEY'}
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
        type_parsing_rules = {PublicKey: lambda name: name + ' PUBLICKEY'}
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
        type_parsing_rules = {PublicKey: lambda name: name + ' PUBLICKEY'}
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
