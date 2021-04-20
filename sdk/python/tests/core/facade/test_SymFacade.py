import unittest
from binascii import unhexlify

from symbolchain.core.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.core.Bip32 import Bip32
from symbolchain.core.CryptoTypes import Hash256, PrivateKey, PublicKey, Signature
from symbolchain.core.facade.SymFacade import SymFacade

from ...test.NemTestUtils import NemTestUtils

YAML_INPUT = '''
- public_key: 87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8
  name: TEST
'''


class SymFacadeTest(unittest.TestCase):
    # region constants

    def test_bip32_constants_are_correct(self):
        self.assertEqual(4343, SymFacade.BIP32_COIN_ID)
        self.assertEqual('ed25519', SymFacade.BIP32_CURVE_NAME)

    def test_key_pair_is_correct(self):
        # Arrange:
        private_key = PrivateKey('E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E')

        # Act:
        key_pair = SymFacade.KeyPair(private_key)

        # Assert:
        self.assertEqual(PublicKey('E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD'), key_pair.public_key)

    def test_can_sign_and_verify(self):
        # Arrange:
        private_key = PrivateKey.random()
        key_pair = SymFacade.KeyPair(private_key)
        message = NemTestUtils.randbytes(21)

        # Act:
        signature = key_pair.sign(message)
        is_verified = SymFacade.Verifier(key_pair.public_key).verify(message, signature)

        # Assert:
        self.assertTrue(is_verified)

    # endregion

    # region constructor

    def test_can_create_around_known_network(self):
        # Act:
        facade = SymFacade('public_test')
        transaction = facade.transaction_factory.create({
            'type': 'transfer',
            'signer_public_key': NemTestUtils.randcryptotype(PublicKey)
        })

        # Assert:
        self.assertEqual('public_test', facade.network.name)

        self.assertEqual(0x4154, transaction.type)
        self.assertEqual(1, transaction.version)

    def test_cannot_create_around_unknown_network(self):
        # Act:
        with self.assertRaises(StopIteration):
            SymFacade('foo')

    # endregion

    # region hash_transaction / sign_transaction

    @staticmethod
    def _create_real_transfer(facade):
        return facade.transaction_factory.create({
            'type': 'transfer',
            'signer_public_key': 'TEST',
            'fee': 1000000,
            'deadline': 41998024783,
            'recipient_address': 'TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA',
            'mosaics': [(0x2CF403E85507F39E, 1000000)]
        })

    @staticmethod
    def _create_real_aggregate(facade):
        aggregate = facade.transaction_factory.create({
            'type': 'aggregateComplete',
            'signer_public_key': 'TEST',
            'fee': 2000000,
            'deadline': 42238390163,
            'transactions_hash': unhexlify('71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE'),
        })
        transfer = facade.transaction_factory.create_embedded({
            'type': 'transfer',
            'signer_public_key': 'TEST',
            'recipient_address': 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
            'mosaics': [(0x2CF403E85507F39E, 1000000)]
        })
        aggregate.transactions.append(transfer)
        return aggregate

    def test_can_hash_transaction(self):
        # Arrange:
        private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
        facade = SymFacade('public_test', AccountDescriptorRepository(YAML_INPUT))

        transaction = self._create_real_transfer(facade)
        transaction.signature = facade.sign_transaction(facade.KeyPair(private_key), transaction).bytes

        # Act:
        hash_value = facade.hash_transaction(transaction)

        # Assert:
        self.assertEqual(Hash256('17EBC7D64F01AA12F55A2B1F50C99B02BC25D06928CEAD1F249A4373B5EB1914'), hash_value)

    def test_can_hash_aggregate_transaction(self):
        # Arrange:
        private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
        facade = SymFacade('public_test', AccountDescriptorRepository(YAML_INPUT))

        transaction = self._create_real_aggregate(facade)
        transaction.signature = facade.sign_transaction(facade.KeyPair(private_key), transaction).bytes

        # Act:
        hash_value = facade.hash_transaction(transaction)

        # Assert:
        self.assertEqual(Hash256('A029FCAC4957C6531B4492F08C211CDDE52C3CD72F2016D6EA37EC96B85606E7'), hash_value)

    def test_can_sign_transaction(self):
        # Arrange:
        private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
        facade = SymFacade('public_test', AccountDescriptorRepository(YAML_INPUT))

        transaction = self._create_real_transfer(facade)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature)

        # Act:
        signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)

        # Assert:
        expected_signature = Signature(''.join([
            '9BC2691B3176149D5E76ED15D83BAB7AC403C754106DFA94E4264F73B92DEC1B',
            '1D514F23C07735EF394DA005AD96C86011EDF49F1FEE56CF3E280B49BEE26608'
        ]))
        self.assertEqual(expected_signature, signature)

    def test_can_sign_aggregate_transaction(self):
        # Arrange:
        private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
        facade = SymFacade('public_test', AccountDescriptorRepository(YAML_INPUT))

        transaction = self._create_real_aggregate(facade)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature)

        # Act:
        signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)

        # Assert:
        expected_signature = Signature(''.join([
            'CD95F7D677A66E980B0B24605049CF405CB1E350ACF65F2BC5427BBBFF531557',
            '487176A464DA6E5D6B17D71ADDD727C3D0C469513C1AB36F27547ED6101B4809'
        ]))
        self.assertEqual(expected_signature, signature)

    def _assert_can_verify_transaction(self, transaction_factory):
        # Arrange:
        private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
        facade = SymFacade('public_test', AccountDescriptorRepository(YAML_INPUT))

        transaction = transaction_factory(facade)

        # Sanity:
        self.assertEqual(Signature.zero().bytes, transaction.signature)

        # Act:
        signature = facade.sign_transaction(facade.KeyPair(private_key), transaction)
        is_verified = facade.verify_transaction(transaction, signature)

        # Assert:
        self.assertTrue(is_verified)

    def test_can_verify_transaction(self):
        self._assert_can_verify_transaction(self._create_real_transfer)

    def test_can_verify_aggregate_transaction(self):
        self._assert_can_verify_transaction(self._create_real_aggregate)

    # endregion

    # region bip32_node_to_key_pair

    def _assert_bip32_child_public_keys(self, passphrase, expected_child_public_keys):
        # Arrange:
        mnemonic_seed = ' '.join([
            'hamster', 'diagram', 'private', 'dutch', 'cause', 'delay', 'private', 'meat', 'slide', 'toddler', 'razor', 'book',
            'happy', 'fancy', 'gospel', 'tennis', 'maple', 'dilemma', 'loan', 'word', 'shrug', 'inflict', 'delay', 'length'
        ])

        # Act:
        root_node = Bip32(SymFacade.BIP32_CURVE_NAME).from_mnemonic(mnemonic_seed, passphrase)

        child_public_keys = []
        for i in range(0, len(expected_child_public_keys)):
            child_node = root_node.derive_path([44, SymFacade.BIP32_COIN_ID, i, 0, 0])
            child_key_pair = SymFacade.bip32_node_to_key_pair(child_node)
            child_public_keys.append(child_key_pair.public_key)

        # Assert:
        self.assertEqual(expected_child_public_keys, child_public_keys)

    def test_can_use_bip32_derivation_without_passphrase(self):
        self._assert_bip32_child_public_keys('', [
            PublicKey('E9CFE9F59CB4393E61B2F42769D9084A644B16883C32C2823E7DF9A3AF83C121'),
            PublicKey('0DE8C3235271E4C9ACF5482F7DFEC1E5C4B20FFC71548703EACF593153F116F9'),
            PublicKey('259866A68A00C325713342232056333D60710E223FC920566B3248B266E899D5')
        ])

    def test_can_use_bip32_derivation_with_passphrase(self):
        self._assert_bip32_child_public_keys('TREZOR', [
            PublicKey('47F4D39D36D11C07735D7BE99220696AAEE7B3EE161D61422220DFE3FF120B15'),
            PublicKey('4BA67E87E8C14F3EB82B3677EA959B56A9D7355705019CED1FCF6C76104E628C'),
            PublicKey('8115D75C13C2D25E7FA3009D03D63F1F32601CDCCA9244D5FDAC74BCF3E892E3')
        ])

    # endregion
