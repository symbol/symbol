import unittest
from functools import reduce

from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.nis1.ImportanceTransferTransaction import ImportanceTransferTransaction
from symbolchain.core.nis1.Network import Network
from symbolchain.tests.test.BasicNisTransactionTest import BasicNisTransactionTest, NisTransactionTestDescriptor

FOO_NETWORK = Network('foo', 0x54)


class ImportanceTransferTransactionTest(BasicNisTransactionTest, unittest.TestCase):
    # region basic

    @staticmethod
    def get_test_descriptor():
        return NisTransactionTestDescriptor(ImportanceTransferTransaction, 'importance-transfer', 0x0801, FOO_NETWORK)

    def test_can_create(self):
        # Act:
        transaction = ImportanceTransferTransaction(FOO_NETWORK)

        # Assert:
        self.assertEqual(0x0801, transaction.type)
        self.assertEqual(0x54000001, transaction.version)
        self.assertEqual(0, transaction.timestamp)
        self.assertEqual(None, transaction.signer)

        self.assertEqual(0, transaction.mode)
        self.assertEqual(None, transaction.remote_account)

        # - properties
        self.assertEqual(150000, transaction.fee)
        self.assertEqual(60 * 60, transaction.deadline)

    # endregion

    # region serialize

    @staticmethod
    def _create_transfer_for_serialization_tests():
        transaction = ImportanceTransferTransaction(FOO_NETWORK)
        transaction.timestamp = 12345
        transaction.signer = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')

        transaction.mode = 2
        transaction.remote_account = PublicKey('9764026AA71A3CD0189990D1B7B8275B8D80863CF271235DFC745F30651E93AA')
        return transaction

    def test_can_serialize(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests()

        # Act:
        buffer = transaction.serialize()

        # Assert:
        expected_buffers = [
            [0x01, 0x08, 0x00, 0x00],  # type
            [0x01, 0x00, 0x00, 0x54],  # version
            [0x39, 0x30, 0x00, 0x00],  # timestamp
            [0x20, 0x00, 0x00, 0x00],  # signer length
            transaction.signer.bytes,  # signer
            [0xF0, 0x49, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00],  # fee
            [0x49, 0x3E, 0x00, 0x00],  # deadline
            [0x02, 0x00, 0x00, 0x00],  # mode
            [0x20, 0x00, 0x00, 0x00],  # remote account length
            transaction.remote_account.bytes,  # remote account
        ]
        expected_buffer = reduce(lambda x, y: bytes(x) + bytes(y), expected_buffers)
        self.assertEqual(expected_buffer, buffer)

    # endregion

    # region string

    def test_can_create_string_representation(self):
        # Arrange:
        transaction = self._create_transfer_for_serialization_tests()

        # Act:
        transaction_str = str(transaction)

        # Assert:
        expected_transaction_str = '\n'.join([
            '          type = 2049 [0x801]',
            '       version = 1409286145 [0x54000001]',
            '     timestamp = 12345 [0x3039]',
            '        signer = D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0',
            '           fee = 150000 [0x249F0]',
            '      deadline = 15945 [0x3E49]',
            '          mode = 2 [0x2]',
            'remote_account = 9764026AA71A3CD0189990D1B7B8275B8D80863CF271235DFC745F30651E93AA'
        ])
        self.assertEqual(expected_transaction_str, transaction_str)

    # endregion
