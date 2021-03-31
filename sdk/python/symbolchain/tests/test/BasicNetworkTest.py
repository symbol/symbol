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

    def _assert_network(self, network, expected_name, expected_identifier):
        self.assertEqual(expected_name, network.name)
        self.assertEqual(expected_identifier, network.identifier)

    @abstractmethod
    def get_test_descriptor(self):
        pass
