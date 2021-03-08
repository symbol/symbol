import unittest
from binascii import unhexlify

from symbolchain.core.Bip32 import Bip32
from symbolchain.core.CryptoTypes import PrivateKey

DETERIMINISTIC_SEED = unhexlify('000102030405060708090A0B0C0D0E0F')
DETERIMINISTIC_MNEMONIC = 'cat swing flag economy stadium alone churn speed unique patch report train'


class Bip32Test(unittest.TestCase):
    # region from_seed

    def test_can_create_root_node(self):
        # Act:
        node = Bip32().from_seed(DETERIMINISTIC_SEED)

        # Assert:
        self._assert_bip32_node(
            node,
            unhexlify('90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB'),
            unhexlify('2B4BE7F19EE27BBF30C667B642D5F4AA69FD169872F8FC3059C08EBAE2EB19E7'))

    def test_can_create_root_node_with_custom_curve_name(self):
        # Act:
        node = Bip32(curve_name='ed25519-keccak').from_seed(DETERIMINISTIC_SEED)

        # Assert:
        self._assert_bip32_node(
            node,
            unhexlify('9CFCA256458AAC0A0550A30DC7639D87364E4323BA61ED41454818E3317BAED0'),
            unhexlify('A3D76D92ACF784D68F4EA2F6DE5507A3520385237A80277132B6C8F3685601B2'))

    def test_can_derive_single_level_node(self):
        # Act:
        node = Bip32().from_seed(DETERIMINISTIC_SEED).derive_one(0)

        # Assert:
        self._assert_bip32_node(
            node,
            unhexlify('8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69'),
            unhexlify('68E0FE46DFB67E368C75379ACEC591DAD19DF3CDE26E63B93A8E704F1DADE7A3'))

    def _assert_well_known_children_from_deterministic_seed(self, child_node0, child_node1):
        self._assert_bip32_node(
            child_node0,
            unhexlify('B8E16D407C8837B46A9445C6417310F3C7A4DCD9B8FF2679C383E6DEF721AC11'),
            unhexlify('BB2724A538CFD64E4366FEB36BB982B954D58EA78F7163451B3B514EDD692159'))
        self._assert_bip32_node(
            child_node1,
            unhexlify('68CA2A058611AAC20CAFB4E1CCD70961E67D8C567390B3CBFC63D0E58BAE7153'),
            unhexlify('8C91D9F5D214A2E80A275E75A165F7022712F7AD52B7ECD45B3B6CC76154B571'))

    def test_can_derive_child_nodes_chained(self):
        # Act:
        node = Bip32().from_seed(DETERIMINISTIC_SEED)
        child_node0 = node.derive_one(44).derive_one(4343).derive_one(0).derive_one(0).derive_one(0)
        child_node1 = node.derive_one(44).derive_one(4343).derive_one(1).derive_one(0).derive_one(0)

        # Assert:
        self._assert_well_known_children_from_deterministic_seed(child_node0, child_node1)

    def test_can_derive_child_nodes_path(self):
        # Act:
        node = Bip32().from_seed(DETERIMINISTIC_SEED)
        child_node0 = node.derive_path([44, 4343, 0, 0, 0])
        child_node1 = node.derive_path([44, 4343, 1, 0, 0])

        # Assert:
        self._assert_well_known_children_from_deterministic_seed(child_node0, child_node1)

    def _assert_bip32_node(self, node, expected_chain_code, expected_private_key):
        self.assertEqual(expected_chain_code, node.chain_code)
        self.assertEqual(PrivateKey(expected_private_key), node.private_key)

    # endregion

    # region from_mnemonic

    def test_can_derive_child_nodes_from_mnemonic_with_password(self):
        # Act:
        node = Bip32().from_mnemonic(DETERIMINISTIC_MNEMONIC, 'TREZOR')
        child_node0 = node.derive_path([44, 4343, 0, 0, 0])
        child_node1 = node.derive_path([44, 4343, 1, 0, 0])
        child_node2 = node.derive_path([44, 4343, 2, 0, 0])

        # Assert:
        self.assertEqual(PrivateKey('1455FB18AB105444763EED593B7CA1C53EF6DDF8BDA1AB7004276FEAB1FCF222'), child_node0.private_key)
        self.assertEqual(PrivateKey('913967B3DFE1E94C50D5C92789DA194644D2A699E5BB75B171A3B68993B82A21'), child_node1.private_key)
        self.assertEqual(PrivateKey('AEC7C0143FC11F26FF5DB020492DACA7C8CF2640D2377AD3C721286472571602'), child_node2.private_key)

    # endregion
