import unittest
from binascii import unhexlify

from symbolchain.core.Bip32 import Bip32
from symbolchain.core.CryptoTypes import Hash256, PrivateKey, PublicKey
from symbolchain.core.facade.SymFacade import SymFacade
from symbolchain.tests.test.NemTestUtils import NemTestUtils


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

        # Assert:
        self.assertEqual('public_test', facade.network.name)

    def test_cannot_create_around_unknown_network(self):
        # Act:
        with self.assertRaises(StopIteration):
            SymFacade('foo')

    # endregion

    # region hash_buffer

    def test_can_hash_buffer(self):
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
        hash_value = SymFacade.hash_buffer(unhexlify(message))

        # Assert:
        self.assertEqual(Hash256('85FEF4EEC0B798E6F4CF29EB5B8D3F3096885EB88865DD62D5D0BD63ADE67384'), hash_value)

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
