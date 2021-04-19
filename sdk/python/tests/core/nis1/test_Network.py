import unittest
from binascii import unhexlify

from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.nis1.Network import Address, Network

from ...test.BasicAddressTest import AddressTestDescriptor, BasicAddressTest
from ...test.BasicNetworkTest import BasicNetworkTest, NetworkTestDescriptor


class AddressTest(BasicAddressTest, unittest.TestCase):
    def get_test_descriptor(self):
        return AddressTestDescriptor(
            Address,
            'TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33',
            unhexlify('988A692D13959918BA9A33BE57B114A0C8BA3E59A3684C977B'))


class NetworkTest(BasicNetworkTest, unittest.TestCase):
    def get_test_descriptor(self):
        return NetworkTestDescriptor(
            Network,
            PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0'),
            [Address('NCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNJABUMH'), Network.MAINNET],
            [Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'), Network.TESTNET])

    def test_correct_predefined_networks_are_registered(self):
        self.assertEqual(2, len(Network.NETWORKS))
        self.assertEqual(['mainnet', 'testnet'], [network.name for network in Network.NETWORKS])

        self._assert_network(Network.MAINNET, 'mainnet', 0x68)
        self._assert_network(Network.TESTNET, 'testnet', 0x98)
