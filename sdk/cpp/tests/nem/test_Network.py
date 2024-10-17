import unittest
from binascii import unhexlify

from symbolchain.CryptoTypes import PublicKey
from symbolchain.nem.Network import Address, Network, NetworkTimestamp

from ..test.BasicAddressTest import AddressTestDescriptor, BasicAddressTest
from ..test.BasicNetworkTest import BasicNetworkTest, NetworkTestDescriptor


class NetworkTimestampTest(unittest.TestCase):
	def test_can_add_seconds(self):
		# Arrange:
		timestamp = NetworkTimestamp(100)

		# Act:
		new_timestamp = timestamp.add_seconds(50)

		# Assert:
		self.assertEqual(100, timestamp.timestamp)
		self.assertEqual(100 + 50, new_timestamp.timestamp)


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
		self.assertEqual('seconds', Network.MAINNET.datetime_converter.time_units)
		self.assertEqual('2015-03-29 00:06:25+00:00', str(Network.MAINNET.datetime_converter.epoch))

		self._assert_network(Network.TESTNET, 'testnet', 0x98)
		self.assertEqual('seconds', Network.TESTNET.datetime_converter.time_units)
		self.assertEqual('2015-03-29 00:06:25+00:00', str(Network.TESTNET.datetime_converter.epoch))
