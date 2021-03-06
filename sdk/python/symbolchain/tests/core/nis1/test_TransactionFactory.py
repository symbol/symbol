import unittest
from binascii import unhexlify
from functools import reduce

from symbolchain.core.CryptoTypes import PublicKey, Signature
from symbolchain.core.nis1.Network import Address, Network
from symbolchain.core.nis1.TransactionFactory import TransactionFactory
from symbolchain.tests.test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)


class MockTransaction:
    # pylint: disable=too-few-public-methods

    def __init__(self, buffer):
        self.buffer = buffer

    def serialize(self):
        return self.buffer


class TransactionFactoryTest(unittest.TestCase):
    # region hasher

    def test_hasher_is_correct(self):
        # Arrange:
        message = ''.join([
            'A6151D4904E18EC288243028CEDA30556E6C42096AF7150D6A7232CA5DBA52BD',
            '2192E23DAA5FA2BEA3D4BD95EFA2389CD193FCD3376E70A5C097B32C1C62C80A',
            'F9D710211545F7CDDDF63747420281D64529477C61E721273CFD78F8890ABB40',
            '70E97BAA52AC8FF61C26D195FC54C077DEF7A3F6F79B36E046C1A83CE9674BA1',
            '983EC2FB58947DE616DD797D6499B0385D5E8A213DB9AD5078A8E0C940FF0CB6',
            'BF92357EA5609F778C3D1FB1E7E36C35DB873361E2BE5C125EA7148EFF4A035B',
            '0CCE880A41190B2E22924AD9D1B82433D9C023924F2311315F07B88BFD428500',
            '47BF3BE785C4CE11C09D7E02065D30F6324365F93C5E7E423A07D754EB314B5F',
            'E9DB4614275BE4BE26AF017ABDC9C338D01368226FE9AF1FB1F815E7317BDBB3',
            '0A0F36DC69'
        ])

        # Act:
        hash_value = TransactionFactory.HASHER(unhexlify(message)).digest()

        # Assert:
        self.assertEqual(unhexlify('4E9E79AB7434F6C7401FB3305D55052EE829B9E46D5D05D43B59FEFB32E9A619'), hash_value)

    # endregion

    # region create

    def test_can_create_transfer(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act:
        transaction = factory.create('transfer')

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x54000001, transaction.version)

    def test_cannot_create_non_transfer(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act + Assert:
        with self.assertRaises(ValueError):
            factory.create('multisig')

    # endregion

    # region create_from_descriptor

    def test_can_create_from_descriptor(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act:
        transaction = factory.create_from_descriptor({'type': 'transfer'})

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x54000001, transaction.version)

    def test_can_create_from_descriptor_with_simple_property_override(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act:
        transaction = factory.create_from_descriptor({
            'type': 'transfer',
            'recipient': 'recipient_name'
        })

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual('recipient_name', transaction.recipient)

    def test_can_create_from_descriptor_with_custom_setter_override(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK)

        # Act:
        transaction = factory.create_from_descriptor({
            'type': 'transfer',
            'message': 'hello world',
        })

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(b'hello world', transaction.message)

    def test_can_create_from_descriptor_with_custom_rule_override(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK, {
            Address: lambda address: address + ' ADDRESS'
        })

        # Act:
        transaction = factory.create_from_descriptor({
            'type': 'transfer',
            'recipient': 'recipient_name'
        })

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual('recipient_name ADDRESS', transaction.recipient)

    def test_can_create_from_descriptor_with_multiple_overrides(self):
        # Arrange:
        factory = TransactionFactory(FOO_NETWORK, {
            Address: lambda address: address + ' ADDRESS',
            PublicKey: lambda address: address + ' PUBLICKEY'
        })

        # Act:
        transaction = factory.create_from_descriptor({
            'type': 'transfer',
            'timestamp': 98765,
            'signer': 'signer_name',
            'recipient': 'recipient_name',
            'message': 'hello world',
        })

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x54000001, transaction.version)
        self.assertEqual(98765, transaction.timestamp)
        self.assertEqual('signer_name PUBLICKEY', transaction.signer)

        self.assertEqual('recipient_name ADDRESS', transaction.recipient)
        self.assertEqual(b'hello world', transaction.message)

    # endregion

    # region attach_signature

    def test_can_attach_signature_to_transaction(self):
        # Arrange:
        transaction = MockTransaction(bytes([0x44, 0x55, 0x98, 0x12, 0x71, 0xAB, 0x72]))
        signature = Signature(NemTestUtils.randbytes(64))

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
