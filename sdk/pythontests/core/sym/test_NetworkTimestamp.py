import datetime
import unittest

from symbolchain.core.sym.NetworkTimestamp import NetworkTimestamp

from ...test.BasicNetworkTimestampTest import BasicNetworkTimestampTest, NetworkTimestampTestDescriptor


class NetworkTimestampTest(BasicNetworkTimestampTest, unittest.TestCase):
    @staticmethod
    def get_test_descriptor():
        epoch_time = datetime.datetime(2021, 3, 16, 0, 6, 25, tzinfo=datetime.timezone.utc)
        return NetworkTimestampTestDescriptor(NetworkTimestamp, epoch_time, 'milliseconds')

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
