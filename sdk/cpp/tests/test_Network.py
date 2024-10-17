import datetime
import unittest

from symbolchain.Network import Network, NetworkLocator
from symbolchain.NetworkTimestamp import NetworkTimestamp, NetworkTimestampDatetimeConverter


class NetworkTest(unittest.TestCase):
	def test_can_convert_network_time_to_datetime(self):
		# Arrange:
		converter = NetworkTimestampDatetimeConverter(datetime.datetime(2022, 3, 16, 0, 6, 25, tzinfo=datetime.timezone.utc), 'minutes')
		network = Network('foo', 0x55, converter, None, None)

		# Act:
		datetime_timestamp = network.to_datetime(NetworkTimestamp(60))

		# Assert:
		self.assertEqual(datetime.datetime(2022, 3, 16, 1, 6, 25, tzinfo=datetime.timezone.utc), datetime_timestamp)

	def test_can_convert_datetime_to_network_time(self):
		# Arrange:
		converter = NetworkTimestampDatetimeConverter(datetime.datetime(2022, 3, 16, 0, 6, 25, tzinfo=datetime.timezone.utc), 'minutes')
		network = Network('foo', 0x55, converter, None, NetworkTimestamp)

		# Act:
		network_timestamp = network.from_datetime(datetime.datetime(2022, 3, 16, 1, 6, 25, tzinfo=datetime.timezone.utc))

		# Assert:
		self.assertEqual(60, network_timestamp.timestamp)

	def test_equality_is_supported(self):
		# Arrange:
		network = Network('foo', 0x55, None, None, None)

		# Act + Assert:
		self.assertEqual(network, Network('foo', 0x55, None, None, None))
		self.assertEqual(network, Network('foo', 0x55, {}, None, None))  # datetime_converter is not compared
		self.assertEqual(network, Network('foo', 0x55, None, int, None))  # address_class is not compared
		self.assertEqual(network, Network('foo', 0x55, None, None, int))  # network_timestamp_class is not compared

		self.assertNotEqual(network, Network('Foo', 0x55, None, None, None))
		self.assertNotEqual(network, Network('foo', 0x54, None, None, None))
		self.assertNotEqual(network, None)

	def test_string_is_supported(self):
		self.assertEqual('foo', str(Network('foo', 0x55, None, None, None)))


PREDEFINED_NETWORKS = [Network('foo', 0x55, None, None, None), Network('bar', 0x37, None, None, None)]


class NetworkLocatorTest(unittest.TestCase):
	def test_can_find_well_known_network_by_name_single(self):
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_name(PREDEFINED_NETWORKS, 'foo'))
		self.assertEqual(PREDEFINED_NETWORKS[1], NetworkLocator.find_by_name(PREDEFINED_NETWORKS, 'bar'))

	def test_can_find_well_known_network_by_name_list(self):
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_name(PREDEFINED_NETWORKS, ['xxx', 'foo']))
		self.assertEqual(PREDEFINED_NETWORKS[1], NetworkLocator.find_by_name(PREDEFINED_NETWORKS, ['bar', 'yyy']))
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_name(PREDEFINED_NETWORKS, ['bar', 'foo']))

	def test_cannot_find_other_network_by_name(self):
		with self.assertRaises(StopIteration):
			NetworkLocator.find_by_name(PREDEFINED_NETWORKS, 'cat')

		with self.assertRaises(StopIteration):
			NetworkLocator.find_by_name(PREDEFINED_NETWORKS, ['cat', 'dog'])

	def test_can_find_well_known_network_by_identifier_single(self):
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, 0x55))
		self.assertEqual(PREDEFINED_NETWORKS[1], NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, 0x37))

	def test_can_find_well_known_network_by_identifier_list(self):
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, [0x88, 0x55]))
		self.assertEqual(PREDEFINED_NETWORKS[1], NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, [0x37, 0x99]))
		self.assertEqual(PREDEFINED_NETWORKS[0], NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, [0x37, 0x55]))

	def test_cannot_find_other_network_by_identifier(self):
		with self.assertRaises(StopIteration):
			NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, 0xFF)

		with self.assertRaises(StopIteration):
			NetworkLocator.find_by_identifier(PREDEFINED_NETWORKS, [0xFF, 0x88])
