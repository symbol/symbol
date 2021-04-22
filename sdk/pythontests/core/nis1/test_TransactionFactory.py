import unittest
from functools import reduce

from symbolchain.core.CryptoTypes import PublicKey, Signature
from symbolchain.core.nis1.Network import Address, Network
from symbolchain.core.nis1.TransactionFactory import TransactionFactory

from ...test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)


class MockTransaction:
    def __init__(self, buffer):
        self.buffer = buffer

    def serialize(self):
        return self.buffer


class TransactionFactoryTest(unittest.TestCase):
    # region create

    def test_can_create_known_transaction_from_descriptor(self):
        # Arrange:
        for transaction_name, transaction_type in [('transfer', 0x0101), ('importance-transfer', 0x0801)]:
            factory = TransactionFactory(FOO_NETWORK)

            # Act:
            transaction = factory.create({'type': transaction_name})

            # Assert:
            self.assertEqual(transaction_type, transaction.type)
            self.assertEqual(0x54000001, transaction.version)

    def test_cannot_create_unknown_transaction_from_descriptor(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act + Assert:
        with self.assertRaises(ValueError):
            factory.create({'type': 'multisig'})

    def test_can_create_known_transaction_with_multiple_overrides(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK, {
            Address: lambda address: address + ' ADDRESS',
            PublicKey: lambda address: address + ' PUBLICKEY'
        })

        # Act:
        transaction = factory.create({
            'type': 'transfer',
            'signer_public_key': 'signer_name',
            'deadline': 98765 + 24 * 60 * 60,
            'recipient_address': 'recipient_name',
            'message': 'hello world',
        })

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x54000001, transaction.version)
        self.assertEqual(98765, transaction.timestamp)
        self.assertEqual('signer_name PUBLICKEY', transaction.signer_public_key)

        self.assertEqual('recipient_name ADDRESS', transaction.recipient_address)
        self.assertEqual(b'hello world', transaction.message)

    # endregion

    # region attach_signature

    def test_can_attach_signature_to_transaction(self):
        # Arrange:
        transaction = MockTransaction(bytes([0x44, 0x55, 0x98, 0x12, 0x71, 0xAB, 0x72]))
        signature = NemTestUtils.randcryptotype(Signature)

        # Act:
        signed_transaction_buffer = TransactionFactory.attach_signature(transaction, signature)

        # Assert:
        expected_buffers = [
            [0x07, 0x00, 0x00, 0x00],  # transaction length
            [0x44, 0x55, 0x98, 0x12, 0x71, 0xAB, 0x72],  # transaction
            [0x40, 0x00, 0x00, 0x00],  # signature length
            signature.bytes  # signature
        ]
        expected_buffer = reduce(lambda x, y: bytes(x) + bytes(y), expected_buffers)
        self.assertEqual(expected_buffer, signed_transaction_buffer)

    # endregion
