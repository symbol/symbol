import unittest
from binascii import hexlify

from symbol_catbuffer.AccountRestrictionFlagsDto import AccountRestrictionFlagsDto
from symbol_catbuffer.EntityTypeDto import EntityTypeDto
from symbol_catbuffer.MosaicRestrictionTypeDto import MosaicRestrictionTypeDto
from symbol_catbuffer.NetworkTypeDto import NetworkTypeDto

from symbolchain.core.CryptoTypes import PublicKey, Signature
from symbolchain.core.sym.IdGenerator import generate_mosaic_id, generate_namespace_id
from symbolchain.core.sym.Network import Address, Network
from symbolchain.core.sym.TransactionFactory import TransactionFactory

from ...test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)
TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randbytes(PublicKey.SIZE)


class TransactionFactoryTest(unittest.TestCase):
    # pylint: disable=too-many-public-methods

    # region create

    def _assert_transfer(self, transaction):
        self.assertEqual(0x4154, transaction.type)
        self.assertEqual(1, transaction.version)
        self.assertEqual(NetworkTypeDto.PUBLIC_TEST, transaction.network)

    def _assert_can_create_known_transaction_from_descriptor(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'transfer',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY
        })

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY, transaction.signer_public_key)

    def test_can_create_known_transaction_from_descriptor(self):
        self._assert_can_create_known_transaction_from_descriptor(lambda factory: factory.create)

    def test_can_create_known_transaction_from_descriptor_embedded(self):
        self._assert_can_create_known_transaction_from_descriptor(lambda factory: factory.create_embedded)

    def _assert_cannot_create_unknown_transaction_from_descriptor(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act + Assert:
        with self.assertRaises(ValueError):
            create_function_accessor(factory)({
                'type': 'xtransfer',
                'signer_public_key': TEST_SIGNER_PUBLIC_KEY
            })

    def test_cannot_create_unknown_transaction_from_descriptor(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor(lambda factory: factory.create)

    def test_cannot_create_unknown_transaction_from_descriptor_embedded(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor(lambda factory: factory.create_embedded)

    def _assert_can_create_known_transaction_with_multiple_overrides(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST, {
            Address: lambda address: address + ' ADDRESS',
            PublicKey: lambda address: address + ' PUBLICKEY'
        })

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'transfer',
            'signer_public_key': 'signer_name',
            'recipient_address': 'recipient_name',
            'message': 'hello world',
            'mosaics': [(0x12345678ABCDEF, 12345)]
        })

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual('signer_name PUBLICKEY', transaction.signer_public_key)

        self.assertEqual('recipient_name ADDRESS', transaction.recipient_address)
        self.assertEqual(b'hello world', transaction.message)
        self.assertEqual([(0x12345678ABCDEF, 12345)], transaction.mosaics)

    def test_can_create_known_transaction_with_multiple_overrides(self):
        self._assert_can_create_known_transaction_with_multiple_overrides(lambda factory: factory.create)

    def test_can_create_known_transaction_with_multiple_overrides_embedded(self):
        self._assert_can_create_known_transaction_with_multiple_overrides(lambda factory: factory.create_embedded)

    # endregion

    # region flags and enums handling

    def _assert_can_create_transaction_with_default_flags_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'accountAddressRestriction',
            'signer_public_key': 'signer_name',
            'restriction_flags': 'address mosaic_id transaction_type outgoing block'
        })

        # Assert:
        Flags = AccountRestrictionFlagsDto
        self.assertEqual(
            [Flags.ADDRESS, Flags.MOSAIC_ID, Flags.TRANSACTION_TYPE, Flags.OUTGOING, Flags.BLOCK],
            transaction.restriction_flags)

    def test_can_create_transaction_with_default_flags_handling(self):
        self._assert_can_create_transaction_with_default_flags_handling(lambda factory: factory.create)

    def test_can_create_transaction_with_default_flags_handling_embedded(self):
        self._assert_can_create_transaction_with_default_flags_handling(lambda factory: factory.create_embedded)

    def _assert_can_create_transaction_with_default_enum_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'mosaicGlobalRestriction',
            'signer_public_key': 'signer_name',
            'mosaic_id': 0x12345,
            'previous_restriction_type': 'le',
            'new_restriction_type': 'gt'
        })

        # Assert:
        self.assertEqual(MosaicRestrictionTypeDto.LE, transaction.previous_restriction_type)
        self.assertEqual(MosaicRestrictionTypeDto.GT, transaction.new_restriction_type)

    def test_can_create_transaction_with_default_enum_handling(self):
        self._assert_can_create_transaction_with_default_enum_handling(lambda factory: factory.create)

    def test_can_create_transaction_with_default_enum_handling_embedded(self):
        self._assert_can_create_transaction_with_default_enum_handling(lambda factory: factory.create_embedded)

    def _assert_can_create_transaction_with_default_enum_array_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'accountOperationRestriction',
            'signer_public_key': 'signer_name',
            'restriction_additions': [
                'transfer_transaction',
                'account_key_link_transaction'
            ],
            'restriction_deletions': [
                'vrf_key_link_transaction',
                'voting_key_link_transaction'
            ]
        })

        # Assert:
        expected_additions = [EntityTypeDto.TRANSFER_TRANSACTION, EntityTypeDto.ACCOUNT_KEY_LINK_TRANSACTION]
        self.assertEqual(expected_additions, transaction.restriction_additions)
        expected_deletions = [EntityTypeDto.VRF_KEY_LINK_TRANSACTION, EntityTypeDto.VOTING_KEY_LINK_TRANSACTION]
        self.assertEqual(expected_deletions, transaction.restriction_deletions)

    def test_can_create_transaction_with_default_enum_array_handling(self):
        self._assert_can_create_transaction_with_default_enum_array_handling(lambda factory: factory.create)

    def test_can_create_transaction_with_default_enum_array_handling_embedded(self):
        self._assert_can_create_transaction_with_default_enum_array_handling(lambda factory: factory.create_embedded)

    # endregion

    # region id autogeneration

    def _assert_can_autogenerate_namespace_registration_root_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'namespaceRegistration',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'registration_type': 'root',
            'duration': 123,
            'name': 'roger'
        })

        # Assert:
        expected_id = generate_namespace_id('roger')
        self.assertEqual(expected_id, transaction.id)

    def test_can_autogenerate_namespace_registration_root_id(self):
        self._assert_can_autogenerate_namespace_registration_root_id(lambda factory: factory.create)

    def test_can_autogenerate_namespace_registration_root_id_embedded(self):
        self._assert_can_autogenerate_namespace_registration_root_id(lambda factory: factory.create_embedded)

    def _assert_can_autogenerate_namespace_registration_child_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'namespaceRegistration',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'registration_type': 'child',
            'parent_id': generate_namespace_id('roger'),
            'name': 'charlie'
        })

        # Assert:
        expected_id = generate_namespace_id('charlie', generate_namespace_id('roger'))
        self.assertEqual(expected_id, transaction.id)

    def test_can_autogenerate_namespace_registration_child_id(self):
        self._assert_can_autogenerate_namespace_registration_child_id(lambda factory: factory.create)

    def test_can_autogenerate_namespace_registration_child_id_embedded(self):
        self._assert_can_autogenerate_namespace_registration_child_id(lambda factory: factory.create_embedded)

    def _assert_can_autogenerate_mosaic_definition_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'mosaicDefinition',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'duration': 1,
            'nonce': 123,
            'flags': 'transferable restrictable',
            'divisibility': 2
        })

        # Assert:
        expected_id = generate_mosaic_id(factory.network.public_key_to_address(PublicKey(TEST_SIGNER_PUBLIC_KEY)), 123)
        self.assertEqual(expected_id, transaction.id)

    def test_can_autogenerate_mosaic_definition_id(self):
        self._assert_can_autogenerate_mosaic_definition_id(lambda factory: factory.create)

    def test_can_autogenerate_mosaic_definition_id_embedded(self):
        self._assert_can_autogenerate_mosaic_definition_id(lambda factory: factory.create_embedded)

    # endregion

    # region attach_signature

    def test_can_attach_signature_to_transaction(self):
        # Arrange:
        factory = TransactionFactory(Network.PUBLIC_TEST)
        transaction = factory.create({
            'type': 'transfer',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY
        })
        signature = NemTestUtils.randcryptotype(Signature)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature)

        # Act:
        signed_transaction_buffer = factory.attach_signature(transaction, signature)

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY, transaction.signer_public_key)

        self.assertEqual(signature.bytes, transaction.signature)

        serialized_transaction_hex = hexlify(transaction.serialize()).decode('utf8').upper()
        expected_buffer = '{{"payload": "{}"}}'.format(serialized_transaction_hex).encode('utf8')
        self.assertEqual(expected_buffer, signed_transaction_buffer)

    # endregion
