import unittest

from symbolchain.core.Network import Network, NetworkLocator


class NetworkTest(unittest.TestCase):
    def test_equality_is_supported(self):
        # Arrange:
        foo_network = Network('foo', 0x55)

        # Act + Assert:
        self.assertEqual(foo_network, Network('foo', 0x55))
        self.assertNotEqual(foo_network, Network('Foo', 0x55))
        self.assertNotEqual(foo_network, Network('foo', 0x54))
        self.assertNotEqual(foo_network, None)

    def test_string_is_supported(self):
        self.assertEqual('foo', str(Network('foo', 0x55)))


PREDEFINED_NETWORKS = [Network('foo', 0x55), Network('bar', 0x37)]


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
