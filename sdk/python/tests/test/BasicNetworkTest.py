from abc import abstractmethod


class NetworkTestDescriptor:
    def __init__(
            self,
            network_class,
            deterministic_public_key,
            expected_mainnet_address_network_pair,
            expected_testnet_address_network_pair):
        # pylint: disable=too-many-function-args

        self.network_class = network_class
        self.deterministic_public_key = deterministic_public_key
        (self.expected_mainnet_address, self.mainnet_network) = expected_mainnet_address_network_pair
        (self.expected_testnet_address, self.testnet_network) = expected_testnet_address_network_pair


def mainnet_accessor(descriptor):
    return descriptor.mainnet_network


def testnet_accessor(descriptor):
    return descriptor.testnet_network


def split_at(buffer, position):
    if position < 0:
        position = len(buffer) + position
    assert 0 <= position < len(buffer)
    return [buffer[0:position], buffer[position:position+1], buffer[position+1:]]


def mutate_byte(buffer, position):
    parts = split_at(buffer, position)
    return parts[0] + (parts[1][0] ^ 0xFF).to_bytes(1, 'little') + parts[2]


class BasicNetworkTest:
    # pylint: disable=no-member

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

    def test_validate_valid_mainnet_address(self):
        self._test_validate_valid_address('mainnet_network')

    def test_validate_valid_testnet_address(self):
        self._test_validate_valid_address('testnet_network')

    def test_validate_invalid_mainnet_address_end(self):
        return self._test_validate_invalid_address('mainnet_network', -1)

    def test_validate_invalid_testnet_address_end(self):
        return self._test_validate_invalid_address('testnet_network', -1)

    def test_validate_invalid_mainnet_address_begin(self):
        return self._test_validate_invalid_address('mainnet_network', 1)

    def test_validate_invalid_testnet_address_begin(self):
        return self._test_validate_invalid_address('testnet_network', 1)

    def _test_validate_valid_address(self, field_name):
        # Arrange:
        test_descriptor = self.get_test_descriptor()
        network = getattr(test_descriptor, field_name)
        address = network.public_key_to_address(test_descriptor.deterministic_public_key)

        # Act + Assert:
        self.assertTrue(network.is_valid_address(address))

    def _test_validate_invalid_address(self, field_name, position):
        # Arrange:
        test_descriptor = self.get_test_descriptor()
        network = getattr(test_descriptor, field_name)
        address = network.public_key_to_address(test_descriptor.deterministic_public_key)
        address.bytes = mutate_byte(address.bytes, position)

        # Act + Assert:
        self.assertFalse(network.is_valid_address(address))

    def _assert_network(self, network, expected_name, expected_identifier):
        self.assertEqual(expected_name, network.name)
        self.assertEqual(expected_identifier, network.identifier)

    @abstractmethod
    def get_test_descriptor(self):
        pass
