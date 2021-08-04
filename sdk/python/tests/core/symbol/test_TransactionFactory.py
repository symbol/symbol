import unittest
from binascii import hexlify

from symbolchain.core.CryptoTypes import PublicKey, Signature
from symbolchain.core.symbol.IdGenerator import generate_mosaic_id, generate_namespace_id
from symbolchain.core.symbol.Network import Address, Network
from symbolchain.core.symbol.TransactionFactory import TransactionFactory
from symbolchain.sc import AccountRestrictionFlags, MosaicRestrictionType, NetworkType, TransactionType, UnresolvedAddress

from ...test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)
TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randcryptotype(PublicKey)


class TransactionFactoryTest(unittest.TestCase):
    # pylint: disable=too-many-public-methods

    @staticmethod
    def create_embedded(factory):
        return factory.create_embedded

    @staticmethod
    def create(factory):
        return factory.create

    # region create

    def _assert_transfer(self, transaction):
        self.assertEqual(TransactionType.TRANSFER, transaction.type_)
        self.assertEqual(0x4154, transaction.type_.value)
        self.assertEqual(1, transaction.version)
        self.assertEqual(NetworkType.TESTNET, transaction.network)

    def _assert_can_create_known_transaction_from_descriptor(self, public_key, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'transfer',
            'signer_public_key': public_key
        })

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)

    def _assert_can_create_known_transaction_from_descriptor_public_key(self, create_function_accessor):
        self._assert_can_create_known_transaction_from_descriptor(TEST_SIGNER_PUBLIC_KEY, create_function_accessor)

    def test_can_create_known_transaction_from_descriptor(self):
        self._assert_can_create_known_transaction_from_descriptor_public_key(self.create)

    def test_can_create_known_transaction_from_descriptor_embedded(self):
        self._assert_can_create_known_transaction_from_descriptor_public_key(self.create_embedded)

    def _assert_cannot_create_unknown_transaction_from_descriptor(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act + Assert:
        with self.assertRaises(ValueError):
            create_function_accessor(factory)({
                'type': 'xtransfer',
                'signer_public_key': TEST_SIGNER_PUBLIC_KEY
            })

    def test_cannot_create_unknown_transaction_from_descriptor(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor(self.create)

    def test_cannot_create_unknown_transaction_from_descriptor_embedded(self):
        self._assert_cannot_create_unknown_transaction_from_descriptor(self.create_embedded)

    def _assert_can_create_known_transaction_with_multiple_overrides(self, create_function_accessor):
        # Arrange:
        # there's subtle difference between the two:
        #  * hint on `hash` is pod:Hash256,
        #    type parsing rule that is getting created is for STRING 'Hash256'
        #  * same goes for `BlockDuration`
        #  * signer_public_key is handled differently in Transaction factory, via hint:
        #    {'signer_public_key': PublicKey} which maps it to SDK type CryptoTypes.PublicKey
        factory = TransactionFactory(Network.TESTNET, {
            'Hash256': lambda x: x + ' a hash',
            'BlockDuration': lambda _: 654321,
            PublicKey: lambda address: address + ' PUBLICKEY'
        })

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'hash_lock',
            'signer_public_key': 'signer_name',
            'hash': 'not really',
            'duration': 'fake duration',
            'mosaic': {'mosaic_id': 0x12345678ABCDEF, 'amount': 12345}
        })

        # Assert:
        self.assertEqual(TransactionType.HASH_LOCK, transaction.type_)
        self.assertEqual(0x4148, transaction.type_.value)
        self.assertEqual(1, transaction.version)
        self.assertEqual(NetworkType.TESTNET, transaction.network)
        self.assertEqual('signer_name PUBLICKEY', transaction.signer_public_key)

        self.assertEqual(654321, transaction.duration)
        self.assertEqual('not really a hash', transaction.hash)
        self.assertEqual(0x12345678ABCDEF, transaction.mosaic.mosaic_id.value)
        self.assertEqual(12345, transaction.mosaic.amount.value)

    def test_can_create_known_transaction_with_multiple_overrides(self):
        self._assert_can_create_known_transaction_with_multiple_overrides(self.create)

    def test_can_create_known_transaction_with_multiple_overrides_embedded(self):
        self._assert_can_create_known_transaction_with_multiple_overrides(self.create_embedded)

    # endregion

    # region flags and enums handling

    def _assert_can_create_transaction_with_default_flags_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'account_address_restriction',
            'signer_public_key': 'signer_name',
            'restriction_flags': 'address mosaic_id transaction_type outgoing block'
        })

        # Assert:
        Flags = AccountRestrictionFlags
        self.assertEqual(
            Flags.ADDRESS | Flags.MOSAIC_ID | Flags.TRANSACTION_TYPE | Flags.OUTGOING | Flags.BLOCK,
            transaction.restriction_flags)

    def test_can_create_transaction_with_default_flags_handling(self):
        self._assert_can_create_transaction_with_default_flags_handling(self.create)

    def test_can_create_transaction_with_default_flags_handling_embedded(self):
        self._assert_can_create_transaction_with_default_flags_handling(self.create_embedded)

    def _assert_can_create_transaction_with_default_enum_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'mosaic_global_restriction',
            'signer_public_key': 'signer_name',
            'mosaic_id': 0x12345,
            'previous_restriction_type': 'le',
            'new_restriction_type': 'gt'
        })

        # Assert:
        self.assertEqual(MosaicRestrictionType.LE, transaction.previous_restriction_type)
        self.assertEqual(MosaicRestrictionType.GT, transaction.new_restriction_type)

    def test_can_create_transaction_with_default_enum_handling(self):
        self._assert_can_create_transaction_with_default_enum_handling(self.create)

    def test_can_create_transaction_with_default_enum_handling_embedded(self):
        self._assert_can_create_transaction_with_default_enum_handling(self.create_embedded)

    def _assert_can_create_transaction_with_default_enum_array_handling(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'account_operation_restriction',
            'signer_public_key': 'signer_name',
            'restriction_additions': [
                'transfer',
                'account_key_link'
            ],
            'restriction_deletions': [
                'vrf_key_link',
                'voting_key_link'
            ]
        })

        # Assert:
        expected_additions = [TransactionType.TRANSFER, TransactionType.ACCOUNT_KEY_LINK]
        self.assertEqual(expected_additions, transaction.restriction_additions)
        expected_deletions = [TransactionType.VRF_KEY_LINK, TransactionType.VOTING_KEY_LINK]
        self.assertEqual(expected_deletions, transaction.restriction_deletions)

    def test_can_create_transaction_with_default_enum_array_handling(self):
        self._assert_can_create_transaction_with_default_enum_array_handling(self.create)

    def test_can_create_transaction_with_default_enum_array_handling_embedded(self):
        self._assert_can_create_transaction_with_default_enum_array_handling(self.create_embedded)

    # endregion

    # region byte array type conversion

    def _assert_can_create_transaction_with_type_conversion(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'account_address_restriction',
            'signer_public_key': 'signer_name',
            'restriction_additions': [
                Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA'),
                Address('DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI')
            ]
        })

        # Assert:
        self.assertEqual(
            [UnresolvedAddress(bytes(range(1, 25))), UnresolvedAddress(bytes(range(26, 50)))],
            transaction.restriction_additions)

    def test_can_create_transaction_with_type_conversion(self):
        self._assert_can_create_transaction_with_type_conversion(self.create)

    def test_can_create_transaction_with_type_conversion_embedded(self):
        self._assert_can_create_transaction_with_type_conversion(self.create_embedded)

    # endregion

    # region id autogeneration

    def _assert_can_autogenerate_namespace_registration_root_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'namespace_registration',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'registration_type': 'root',
            'duration': 123,
            'name': 'roger'
        })

        # Assert:
        expected_id = generate_namespace_id('roger')
        self.assertEqual(expected_id, transaction.id.value)

    def test_can_autogenerate_namespace_registration_root_id(self):
        self._assert_can_autogenerate_namespace_registration_root_id(self.create)

    def test_can_autogenerate_namespace_registration_root_id_embedded(self):
        self._assert_can_autogenerate_namespace_registration_root_id(self.create_embedded)

    def _assert_can_autogenerate_namespace_registration_child_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'namespace_registration',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'registration_type': 'child',
            'parent_id': generate_namespace_id('roger'),
            'name': 'charlie'
        })

        # Assert:
        expected_id = generate_namespace_id('charlie', generate_namespace_id('roger'))
        self.assertEqual(expected_id, transaction.id.value)

    def test_can_autogenerate_namespace_registration_child_id(self):
        self._assert_can_autogenerate_namespace_registration_child_id(self.create)

    def test_can_autogenerate_namespace_registration_child_id_embedded(self):
        self._assert_can_autogenerate_namespace_registration_child_id(self.create_embedded)

    def _assert_can_autogenerate_mosaic_definition_id(self, create_function_accessor):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)

        # Act:
        transaction = create_function_accessor(factory)({
            'type': 'mosaic_definition',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
            'duration': 1,
            'nonce': 123,
            'flags': 'transferable restrictable',
            'divisibility': 2
        })

        # Assert:
        expected_id = generate_mosaic_id(factory.network.public_key_to_address(PublicKey(TEST_SIGNER_PUBLIC_KEY)), 123)
        self.assertEqual(expected_id, transaction.id.value)

    def test_can_autogenerate_mosaic_definition_id(self):
        self._assert_can_autogenerate_mosaic_definition_id(self.create)

    def test_can_autogenerate_mosaic_definition_id_embedded(self):
        self._assert_can_autogenerate_mosaic_definition_id(self.create_embedded)

    # endregion

    # region attach_signature

    def test_can_attach_signature_to_transaction(self):
        # Arrange:
        factory = TransactionFactory(Network.TESTNET)
        transaction = factory.create({
            'type': 'transfer',
            'signer_public_key': TEST_SIGNER_PUBLIC_KEY
        })
        signature = NemTestUtils.randcryptotype(Signature)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

        # Act:
        signed_transaction_buffer = factory.attach_signature(transaction, signature)

        # Assert:
        self._assert_transfer(transaction)
        self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)

        self.assertEqual(signature.bytes, transaction.signature.bytes)

        serialized_transaction_hex = hexlify(transaction.serialize()).decode('utf8').upper()
        expected_buffer = f'{{"payload": "{serialized_transaction_hex}"}}'.encode('utf8')
        self.assertEqual(expected_buffer, signed_transaction_buffer)

    # endregion
