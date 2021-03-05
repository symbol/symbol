import unittest

from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.sym.Network import Network
from symbolchain.tests.test.BasicNetworkTest import BasicNetworkTest, NetworkTestDescriptor


class NetworkTest(BasicNetworkTest, unittest.TestCase):
    def get_test_descriptor(self):
        return NetworkTestDescriptor(
            Network,
            PublicKey('C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42F68C7CA1772FE8A0'),
            [b'NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY', Network.PUBLIC],
            [b'TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA', Network.PUBLIC_TEST])

    def test_correct_predefined_networks_are_registered(self):
        self.assertEqual(4, len(Network.NETWORKS))
        self.assertEqual(['public', 'private', 'public_test', 'private_test'], [network.name for network in Network.NETWORKS])

        self._assert_network(Network.PUBLIC, 'public', 0x68)
        self._assert_network(Network.PRIVATE, 'private', 0x78)
        self._assert_network(Network.PUBLIC_TEST, 'public_test', 0x98)
        self._assert_network(Network.PRIVATE_TEST, 'private_test', 0xA8)
