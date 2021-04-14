import datetime
import unittest

from symbolchain.core.NetworkTimestamp import NetworkTimestamp, NetworkTimestampDatetimeConverter


# needed because NetworkTimestamp is abstract
class ConcreteNetworkTimestamp(NetworkTimestamp):
    def add_seconds(self, count):
        return ConcreteNetworkTimestamp(self.timestamp + 5 * count)


class NetworkTimestampTest(unittest.TestCase):
    def test_can_create_epochal_timestamp(self):
        # Act:
        timestamp = ConcreteNetworkTimestamp(0)

        # Assert:
        self.assertTrue(timestamp.epochal)
        self.assertEqual(0, timestamp.timestamp)

    def test_can_create_non_epochal_timestamp(self):
        # Act:
        timestamp = ConcreteNetworkTimestamp(123)

        # Assert:
        self.assertFalse(timestamp.epochal)
        self.assertEqual(123, timestamp.timestamp)

    def test_can_add_minutes(self):
        # Arrange:
        timestamp = ConcreteNetworkTimestamp(100)

        # Act:
        new_timestamp = timestamp.add_minutes(50)

        # Assert:
        self.assertEqual(100, timestamp.timestamp)
        self.assertEqual(100 + 60 * 5 * 50, new_timestamp.timestamp)

    def test_can_add_hours(self):
        # Arrange:
        timestamp = ConcreteNetworkTimestamp(100)

        # Act:
        new_timestamp = timestamp.add_hours(50)

        # Assert:
        self.assertEqual(100, timestamp.timestamp)
        self.assertEqual(100 + 60 * 60 * 5 * 50, new_timestamp.timestamp)

    def test_equality_is_supported(self):
        # Arrange:
        timestamp = ConcreteNetworkTimestamp(123)

        # Act + Assert:
        self.assertEqual(timestamp, ConcreteNetworkTimestamp(123))
        self.assertNotEqual(timestamp, ConcreteNetworkTimestamp(122))
        self.assertNotEqual(timestamp, ConcreteNetworkTimestamp(124))
        self.assertNotEqual(timestamp, None)

    def test_string_is_supported(self):
        self.assertEqual('123', str(ConcreteNetworkTimestamp(123)))


def create_converter():
    return NetworkTimestampDatetimeConverter(datetime.datetime(2020, 1, 2, 3), 'hours')


class NetworkTimestampDatetimeConverterTest(unittest.TestCase):
    def test_can_convert_epochal_timestamp_to_datetime(self):
        # Arrange:
        converter = create_converter()

        # Act:
        utc_timestamp = converter.to_datetime(0)

        # Assert:
        self.assertEqual(datetime.datetime(2020, 1, 2, 3), utc_timestamp)

    def test_can_convert_non_epochal_timestamp_to_datetime(self):
        # Arrange:
        converter = create_converter()

        # Act:
        utc_timestamp = converter.to_datetime(5)

        # Assert:
        self.assertEqual(datetime.datetime(2020, 1, 2, 3 + 5), utc_timestamp)

    def test_cannot_convert_datetime_before_epochal_timestamp(self):
        # Arrange:
        converter = create_converter()

        # Act + Assert::
        with self.assertRaises(ValueError):
            converter.to_difference(datetime.datetime(2020, 1, 2, 2))

    def test_can_convert_datetime_to_epochal_timestamp(self):
        # Arrange:
        converter = create_converter()

        # Act:
        raw_timestamp = converter.to_difference(datetime.datetime(2020, 1, 2, 3))

        # Assert:
        self.assertEqual(0, raw_timestamp)

    def test_can_convert_datetime_to_non_epochal_timestamp(self):
        # Arrange:
        converter = create_converter()

        # Act:
        raw_timestamp = converter.to_difference(datetime.datetime(2020, 1, 2, 3 + 5))

        # Assert:
        self.assertEqual(5, raw_timestamp)
