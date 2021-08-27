import unittest
from binascii import unhexlify

from symbolchain.core.CryptoTypes import Hash256, PublicKey
from symbolchain.core.symbol.Network import Address, Network

from ...test.BasicAddressTest import AddressTestDescriptor, BasicAddressTest
from ...test.BasicNetworkTest import BasicNetworkTest, NetworkTestDescriptor

MAINNET_GENERATION_HASH_SEED = Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6')
TESTNET_GENERATION_HASH_SEED = Hash256('3B5E1FA6445653C971A50687E75E6D09FB30481055E3990C84B25E9222DC1155')


class AddressTest(BasicAddressTest, unittest.TestCase):
    def get_test_descriptor(self):
        return AddressTestDescriptor(
            Address,
            'TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA',
            unhexlify('985783F7A83BE5D8084C6DA3B366CAAFACC129F36A3EA90C'))


class NetworkTest(BasicNetworkTest, unittest.TestCase):
    def get_test_descriptor(self):
        return NetworkTestDescriptor(
            Network,
            PublicKey('C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42F68C7CA1772FE8A0'),
            [Address('NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY'), Network.MAINNET],
            [Address('TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA'), Network.TESTNET])

    def test_correct_predefined_networks_are_registered(self):
        self.assertEqual(2, len(Network.NETWORKS))
        self.assertEqual(['mainnet', 'testnet'], [network.name for network in Network.NETWORKS])

        self._assert_network(Network.MAINNET, 'mainnet', 0x68)
        self.assertEqual(MAINNET_GENERATION_HASH_SEED, Network.MAINNET.generation_hash_seed)

        self._assert_network(Network.TESTNET, 'testnet', 0x98)
        self.assertEqual(TESTNET_GENERATION_HASH_SEED, Network.TESTNET.generation_hash_seed)
