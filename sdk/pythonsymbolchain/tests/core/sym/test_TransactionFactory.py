import unittest
from binascii import hexlify

from symbol_catbuffer.NetworkTypeDto import NetworkTypeDto

from symbolchain.core.CryptoTypes import PublicKey, Signature
from symbolchain.core.sym.Network import Address, Network
from symbolchain.core.sym.TransactionFactory import TransactionFactory
from symbolchain.tests.test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)
TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randbytes(PublicKey.SIZE)


class TransactionFactoryTest(unittest.TestCase):
    # region create

    def _assert_transfer(self, transaction):
        self.assertEqual(0x4154, transaction.type)
        self.assertEqual(1, transaction.version)
        self.assertEqual(NetworkTypeDto.PUBLIC_TEST, transaction.network)

    def _assert_can_create_known_transaction_from_descriptor(self, transaction_type, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': transaction_type,
            'signerPublicKey': TEST_SIGNER_PUBLIC_KEY
        })

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY, transaction.signerPublicKey)

    def test_can_create_known_transaction_from_descriptor(self):
        self._assert_can_create_known_transaction_from_descriptor('transfer', lambda factory: factory.create)

    def test_can_create_known_transaction_from_descriptor_embedded(self):
        self._assert_can_create_known_transaction_from_descriptor('embeddedTransfer', lambda factory: factory.create_embedded)

    def _assert_cannot_create_unknown_transaction_from_descriptor(self, transaction_type, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act + Assert:
        with self.assertRaises(ValueError):
            create_function_accessor(factory)({
                'type': transaction_type,
                'signerPublicKey': TEST_SIGNER_PUBLIC_KEY
            })

    def test_cannot_create_unknown_transaction_from_descriptor(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor('embeddedTransfer', lambda factory: factory.create)

    def test_cannot_create_unknown_transaction_from_descriptor_embedded(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor('transfer', lambda factory: factory.create_embedded)

    def _assert_can_create_known_transaction_with_multiple_overrides(self, transaction_type, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST, {
            Address: lambda address: address + ' ADDRESS',
            PublicKey: lambda address: address + ' PUBLICKEY'
        })

        # Act:
        transaction = create_function_accessor(factory)({
            'type': transaction_type,
            'signerPublicKey': 'signer_name',
            'recipientAddress': 'recipient_name',
            'message': 'hello world',
            'mosaics': [(0x12345678ABCDEF, 12345)]
        })

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual('signer_name PUBLICKEY', transaction.signerPublicKey)

        self.assertEqual('recipient_name ADDRESS', transaction.recipientAddress)
        self.assertEqual('hello world', transaction.message)
        self.assertEqual([(0x12345678ABCDEF, 12345)], transaction.mosaics)

    def test_can_create_known_transaction_with_multiple_overrides(self):
        self._assert_can_create_known_transaction_with_multiple_overrides('transfer', lambda factory: factory.create)

    def test_can_create_known_transaction_with_multiple_overridesembedded(self):
        self._assert_can_create_known_transaction_with_multiple_overrides('embeddedTransfer', lambda factory: factory.create_embedded)

    # endregion

    # region attach_signature

    def test_can_attach_signature_to_transaction(self):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)
        transaction = factory.create({
            'type': 'transfer',
            'signerPublicKey': TEST_SIGNER_PUBLIC_KEY
        })
        signature = NemTestUtils.randcryptotype(Signature)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature)

        # Act:
        signed_transaction_buffer = factory.attach_signature(transaction, signature)

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY, transaction.signerPublicKey)

        self.assertEqual(signature.bytes, transaction.signature)

        serialized_transaction_hex = hexlify(transaction.serialize()).decode('utf8').upper()
        expected_buffer = '{{"payload": "{}"}}'.format(serialized_transaction_hex).encode('utf8')
        self.assertEqual(expected_buffer, signed_transaction_buffer)

    # endregion
