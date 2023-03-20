import unittest
from binascii import unhexlify

from symbolchain.CryptoTypes import Hash256, PublicKey
from symbolchain.symbol.Network import Address, Network, NetworkTimestamp

from ..test.BasicAddressTest import AddressTestDescriptor, BasicAddressTest
from ..test.BasicNetworkTest import BasicNetworkTest, NetworkTestDescriptor

MAINNET_GENERATION_HASH_SEED = Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6')
TESTNET_GENERATION_HASH_SEED = Hash256('49D6E1CE276A85B70EAFE52349AACCA389302E7A9754BCF1221E79494FC665A4')


class NetworkTimestampTest(unittest.TestCase):
	def test_can_add_milliseconds(self):
		# Arrange:
		timestamp = NetworkTimestamp(100)

		# Act:
		new_timestamp = timestamp.add_milliseconds(50)

		# Assert:
		self.assertEqual(100, timestamp.timestamp)
		self.assertEqual(100 + 50, new_timestamp.timestamp)

	def test_can_add_seconds(self):
		# Arrange:
		timestamp = NetworkTimestamp(100)

		# Act:
		new_timestamp = timestamp.add_seconds(50)

		# Assert:
		self.assertEqual(100, timestamp.timestamp)
		self.assertEqual(100 + 50 * 1000, new_timestamp.timestamp)


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
		self.assertEqual('milliseconds', Network.MAINNET.datetime_converter.time_units)
		self.assertEqual('2021-03-16 00:06:25+00:00', str(Network.MAINNET.datetime_converter.epoch))

		self._assert_network(Network.TESTNET, 'testnet', 0x98)
		self.assertEqual(TESTNET_GENERATION_HASH_SEED, Network.TESTNET.generation_hash_seed)
		self.assertEqual('milliseconds', Network.TESTNET.datetime_converter.time_units)
		self.assertEqual('2022-10-31 21:07:47+00:00', str(Network.TESTNET.datetime_converter.epoch))
