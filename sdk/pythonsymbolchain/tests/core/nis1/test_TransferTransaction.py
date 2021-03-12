import unittest
from functools import reduce

from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.nis1.Network import Address, Network
from symbolchain.core.nis1.TransferTransaction import TransferTransaction
from symbolchain.tests.test.BasicNisTransactionTest import BasicNisTransactionTest, NisTransactionTestDescriptor

FOO_NETWORK = Network('foo', 0x54)


class TransferTransactionTest(BasicNisTransactionTest, unittest.TestCase):
    # region basic

    @staticmethod
    def get_test_descriptor():
        return NisTransactionTestDescriptor(TransferTransaction, 'transfer', 0x0101, FOO_NETWORK)

    def test_can_create(self):
        # Act:
        transaction = TransferTransaction(FOO_NETWORK)

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x54000001, transaction.version)
        self.assertEqual(0, transaction.timestamp)
        self.assertEqual(None, transaction.signer)

        self.assertEqual(None, transaction.recipient)
        self.assertEqual(0, transaction.amount)
        self.assertEqual(None, transaction.message)

        # - properties
        self.assertEqual(50000, transaction.fee)
        self.assertEqual(60 * 60, transaction.deadline)

    # endregion

    # region properties

    def test_fee_is_updated_with_amount(self):
        # Arrange:
        transaction = TransferTransaction(FOO_NETWORK)

        # Act:
        transaction.amount = 15 * 10000000000 - 1

        # Assert:
        self.assertEqual(700000, transaction.fee)

    def test_fee_is_updated_with_message(self):
        # Arrange:
        transaction = TransferTransaction(FOO_NETWORK)

        # Act:
        transaction.message = [0, 1, 2] * 32 + [3] * 31

        # Assert:
        self.assertEqual(250000, transaction.fee)

    def test_fee_is_updated_with_amount_and_message(self):
        # Arrange:
        transaction = TransferTransaction(FOO_NETWORK)

        # Act:
        transaction.amount = 15 * 10000000000 - 1
        transaction.message = [0, 1, 2] * 32 + [3] * 31

        # Assert:
        self.assertEqual(900000, transaction.fee)

    def test_can_set_message_as_bytes(self):
        # Arrange:
        transaction = TransferTransaction(FOO_NETWORK)

        # Act:
        transaction.message = b'Hello!'

        # Assert:
        self.assertEqual(b'Hello!', transaction.message)

    def test_can_set_message_as_string(self):
        # Arrange:
        transaction = TransferTransaction(FOO_NETWORK)

        # Act:
        transaction.message = 'Hello!'

        # Assert:
        self.assertEqual(b'Hello!', transaction.message)

    # endregion

    # region serialize

    @staticmethod
    def _create_transfer_for_serialization_tests(include_message=False):
        transaction = TransferTransaction(FOO_NETWORK)
        transaction.timestamp = 12345
        transaction.signer = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')

        transaction.recipient = Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33')
        transaction.amount = 15 * 10000000000 - 1

        if include_message:
            transaction.message = [0x44, 0x55, 0x98, 0x12, 0x71, 0xAB, 0x72]

        return transaction

    def test_can_serialize_without_message(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests(False)

        # Act:
        buffer = transaction.serialize()

        # Assert:
        expected_buffers = [
            [0x01, 0x01, 0x00, 0x00],  # type
            [0x01, 0x00, 0x00, 0x54],  # version
            [0x39, 0x30, 0x00, 0x00],  # timestamp
            [0x20, 0x00, 0x00, 0x00],  # signer length
            transaction.signer.bytes,  # signer
            [0x60, 0xAE, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00],  # fee
            [0x49, 0x3E, 0x00, 0x00],  # deadline
            [0x28, 0x00, 0x00, 0x00],  # recipient length
            str(transaction.recipient).encode('utf8'),  # recipient
            [0xFF, 0x5B, 0xB2, 0xEC, 0x22, 0x00, 0x00, 0x00],  # amount
            [0x00, 0x00, 0x00, 0x00]  # message length
        ]
        expected_buffer = reduce(lambda x, y: bytes(x) + bytes(y), expected_buffers)
        self.assertEqual(expected_buffer, buffer)

    def test_can_serialize_with_message(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests(True)

        # Act:
        buffer = transaction.serialize()

        # Assert:
        expected_buffers = [
            [0x01, 0x01, 0x00, 0x00],  # type
            [0x01, 0x00, 0x00, 0x54],  # version
            [0x39, 0x30, 0x00, 0x00],  # timestamp
            [0x20, 0x00, 0x00, 0x00],  # signer length
            transaction.signer.bytes,  # signer
            [0xB0, 0x71, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00],  # fee
            [0x49, 0x3E, 0x00, 0x00],  # deadline
            [0x28, 0x00, 0x00, 0x00],  # recipient length
            str(transaction.recipient).encode('utf8'),  # recipient
            [0xFF, 0x5B, 0xB2, 0xEC, 0x22, 0x00, 0x00, 0x00],  # amount
            [0x0F, 0x00, 0x00, 0x00],  # message length (including message header)
            [0x01, 0x00, 0x00, 0x00],  # message type
            [0x07, 0x00, 0x00, 0x00],  # message length
            transaction.message
        ]
        expected_buffer = reduce(lambda x, y: bytes(x) + bytes(y), expected_buffers)
        self.assertEqual(expected_buffer, buffer)

    # endregion

    # region string

    def test_can_create_string_representation_without_message(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests(False)

        # Act:
        transaction_str = str(transaction)

        # Assert:
        expected_transaction_str = '\n'.join([
            '     type = 257 [0x101]',
            '  version = 1409286145 [0x54000001]',
            'timestamp = 12345 [0x3039]',
            '   signer = D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0',
            '      fee = 700000 [0xAAE60]',
            ' deadline = 15945 [0x3E49]',
            'recipient = TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33',
            '   amount = 149999999999 [0x22ECB25BFF]',
            '  message = None'
        ])
        self.assertEqual(expected_transaction_str, transaction_str)

    def test_can_create_string_representation_with_message(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests(True)

        # Act:
        transaction_str = str(transaction)

        # Assert:
        expected_transaction_str = '\n'.join([
            '     type = 257 [0x101]',
            '  version = 1409286145 [0x54000001]',
            'timestamp = 12345 [0x3039]',
            '   signer = D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0',
            '      fee = 750000 [0xB71B0]',
            ' deadline = 15945 [0x3E49]',
            'recipient = TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33',
            '   amount = 149999999999 [0x22ECB25BFF]',
            '  message = 4455981271AB72'
        ])
        self.assertEqual(expected_transaction_str, transaction_str)

    def test_can_create_string_representation_with_message_string(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests(True)
        transaction.message = 'Hello!!'

        # Act:
        transaction_str = str(transaction)

        # Assert:
        expected_transaction_str = '\n'.join([
            '     type = 257 [0x101]',
            '  version = 1409286145 [0x54000001]',
            'timestamp = 12345 [0x3039]',
            '   signer = D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0',
            '      fee = 750000 [0xB71B0]',
            ' deadline = 15945 [0x3E49]',
            'recipient = TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33',
            '   amount = 149999999999 [0x22ECB25BFF]',
            '  message = Hello!!'
        ])
        self.assertEqual(expected_transaction_str, transaction_str)

    # endregion
