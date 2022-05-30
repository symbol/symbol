import datetime
from abc import abstractmethod

from symbolchain.NetworkTimestamp import NetworkTimestamp


class NetworkTestDescriptor:
	def __init__(
		self,
		network_class,
		deterministic_public_key,
		mainnet_address_network_pair,
		testnet_address_network_pair):
		# pylint: disable=too-many-function-args

		self.network_class = network_class
		self.deterministic_public_key = deterministic_public_key
		(self.expected_mainnet_address, self.mainnet_network) = mainnet_address_network_pair
		(self.expected_testnet_address, self.testnet_network) = testnet_address_network_pair


def mainnet_accessor(descriptor):
	return descriptor.mainnet_network


def testnet_accessor(descriptor):
	return descriptor.testnet_network


def split_at(buffer, position):
	if position < 0:
		position = len(buffer) + position

	assert 0 <= position < len(buffer)
	return [buffer[0:position], buffer[position:position + 1], buffer[position + 1:]]


def mutate_byte(buffer, position):
	parts = split_at(buffer, position)
	return parts[0] + (parts[1][0] ^ 0xFF).to_bytes(1, 'little') + parts[2]


class BasicNetworkTest:
	# pylint: disable=no-member

	# region public_key_to_address

	def test_can_convert_mainnet_public_key_to_address(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		# Act:
		address = test_descriptor.mainnet_network.public_key_to_address(test_descriptor.deterministic_public_key)

		# Assert:
		self.assertEqual(test_descriptor.expected_mainnet_address, address)

	def test_can_convert_testnet_public_key_to_address(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		# Act:
		address = test_descriptor.testnet_network.public_key_to_address(test_descriptor.deterministic_public_key)

		# Assert:
		self.assertEqual(test_descriptor.expected_testnet_address, address)

	# endregion

	# region is_valid_address[_string]

	def test_can_validate_valid_mainnet_address(self):
		self._test_can_validate_valid_address('mainnet_network')

	def test_can_validate_valid_testnet_address(self):
		self._test_can_validate_valid_address('testnet_network')

	def test_cannot_validate_invalid_mainnet_address_begin(self):
		self._test_cannot_validate_invalid_address('mainnet_network', 1)

	def test_cannot_validate_invalid_testnet_address_begin(self):
		self._test_cannot_validate_invalid_address('testnet_network', 1)

	def test_cannot_validate_invalid_mainnet_address_end(self):
		self._test_cannot_validate_invalid_address('mainnet_network', -1)

	def test_cannot_validate_invalid_testnet_address_end(self):
		self._test_cannot_validate_invalid_address('testnet_network', -1)

	def test_cannot_validate_invalid_mainnet_address_string_invalid_size(self):
		self._test_cannot_validate_invalid_address_string('mainnet_network', lambda address_string: f'{address_string}A')
		self._test_cannot_validate_invalid_address_string('mainnet_network', lambda address_string: address_string[:-1])

	def test_cannot_validate_invalid_testnet_address_string_invalid_size(self):
		self._test_cannot_validate_invalid_address_string('testnet_network', lambda address_string: f'{address_string}A')
		self._test_cannot_validate_invalid_address_string('testnet_network', lambda address_string: address_string[:-1])

	def test_cannot_validate_invalid_mainnet_address_string_invalid_char(self):
		self._test_cannot_validate_invalid_address_string(
			'mainnet_network',
			lambda address_string: f'{address_string[0:10]}@{address_string[11:]}')

	def test_cannot_validate_invalid_testnet_address_string_invalid_char(self):
		self._test_cannot_validate_invalid_address_string(
			'testnet_network',
			lambda address_string: f'{address_string[0:10]}@{address_string[11:]}')

	def _test_can_validate_valid_address(self, field_name):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = getattr(test_descriptor, field_name)
		address = network.public_key_to_address(test_descriptor.deterministic_public_key)

		# Act + Assert:
		self.assertTrue(network.is_valid_address(address))
		self.assertTrue(network.is_valid_address_string(str(address)))

	def _test_cannot_validate_invalid_address(self, field_name, position):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = getattr(test_descriptor, field_name)
		address = network.public_key_to_address(test_descriptor.deterministic_public_key)
		address.bytes = mutate_byte(address.bytes, position)

		# Act + Assert:
		self.assertFalse(network.is_valid_address(address))
		self.assertFalse(network.is_valid_address_string(str(address)))

	def _test_cannot_validate_invalid_address_string(self, field_name, mutator):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = getattr(test_descriptor, field_name)
		address = network.public_key_to_address(test_descriptor.deterministic_public_key)
		address_string = mutator(str(address))

		# Act + Assert:
		self.assertFalse(network.is_valid_address_string(address_string))

	# endregion

	# region to_datetime

	def test_can_convert_epochal_timestamp_to_datetime(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = test_descriptor.mainnet_network
		epoch = network.datetime_converter.epoch

		# Act:
		datetime_timestamp = network.to_datetime(NetworkTimestamp(0))

		# Assert:
		self.assertEqual(epoch, datetime_timestamp)

	def test_can_convert_non_epochal_timestamp_to_datetime(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = test_descriptor.mainnet_network
		epoch = network.datetime_converter.epoch

		# Act:
		datetime_timestamp = network.to_datetime(NetworkTimestamp(123))

		# Assert:
		self.assertEqual(epoch + self._get_time_delta(123), datetime_timestamp)

	# endregion

	# region from_datetime

	def test_can_convert_datetime_to_epochal_timestamp(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = test_descriptor.mainnet_network
		epoch = network.datetime_converter.epoch

		# Act:
		network_timestamp = network.from_datetime(epoch)

		# Assert:
		self.assertTrue(network_timestamp.is_epochal)
		self.assertEqual(0, network_timestamp.timestamp)

	def test_can_convert_datetime_to_non_epochal_timestamp(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		network = test_descriptor.mainnet_network
		epoch = network.datetime_converter.epoch

		# Act:
		network_timestamp = network.from_datetime(epoch + self._get_time_delta(123))

		# Assert:
		self.assertFalse(network_timestamp.is_epochal)
		self.assertEqual(123, network_timestamp.timestamp)

	# endregion

	def _assert_network(self, network, expected_name, expected_identifier):
		self.assertEqual(expected_name, network.name)
		self.assertEqual(expected_identifier, network.identifier)

	def _get_time_delta(self, count):
		time_units = self.get_test_descriptor().mainnet_network.datetime_converter.time_units
		return datetime.timedelta(**{time_units: count})

	@abstractmethod
	def get_test_descriptor(self):
		pass
