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

    def test_cannot_lookup_value_when_descriptor_does_not_contain_key(self):
        # Arrange:
        processor = self._create_processor()

        # Act + Assert:
        with self.assertRaises(ValueError):
            processor.lookup_value('foo')

    def test_can_lookup_value_without_type_hint(self):
        # Arrange:
        processor = self._create_processor()

        # Act:
        message = processor.lookup_value('message')

        # Assert:
        self.assertEqual('hello world', message)

    def test_can_lookup_value_with_type_hint_but_without_custom_rule(self):
        # Arrange:
        processor = self._create_processor()

        # Act:
        timestamp = processor.lookup_value('timestamp')

        # Assert:
        self.assertEqual(12345, timestamp)

    def test_can_lookup_value_with_type_hint_and_with_custom_rule(self):
        # Arrange:
        processor = self._create_processor()

        # Act:
        signer = processor.lookup_value('signer')

        # Assert:
        self.assertEqual('signer_name PUBLICKEY', signer)

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

    # region set_type_hints

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
