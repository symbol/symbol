import datetime
from abc import abstractmethod

CUSTOM_EPOCH_TIME = datetime.datetime(2021, 1, 1, 0, 0, 0, tzinfo=datetime.timezone.utc)


class NetworkTimestampTestDescriptor:
    def __init__(self, network_timestamp_class, epoch, time_units):
        self.network_timestamp_class = network_timestamp_class
        self.epoch = epoch
        self.time_units = time_units


class BasicNetworkTimestampTest:
    # pylint: disable=no-member

    # region to_datetime

    def test_can_convert_epochal_timestamp_to_datetime(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        utc_timestamp = test_descriptor.network_timestamp_class(0).to_datetime()

        # Assert:
        self.assertEqual(test_descriptor.epoch, utc_timestamp)

    def test_can_convert_non_epochal_timestamp_to_datetime(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        utc_timestamp = test_descriptor.network_timestamp_class(123).to_datetime()

        # Assert:
        self.assertEqual(test_descriptor.epoch + self._get_time_delta(123), utc_timestamp)

    def test_can_convert_epochal_timestamp_to_datetime_custom_epoch(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        utc_timestamp = test_descriptor.network_timestamp_class(0).to_datetime(CUSTOM_EPOCH_TIME)

        # Assert:
        self.assertEqual(CUSTOM_EPOCH_TIME, utc_timestamp)

    def test_can_convert_non_epochal_timestamp_to_datetime_custom_epoch(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        utc_timestamp = test_descriptor.network_timestamp_class(123).to_datetime(CUSTOM_EPOCH_TIME)

        # Assert:
        self.assertEqual(CUSTOM_EPOCH_TIME + self._get_time_delta(123), utc_timestamp)

    # endregion

    # region from_datetime

    def test_can_convert_datetime_to_epochal_timestamp(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        timestamp = test_descriptor.network_timestamp_class.from_datetime(test_descriptor.epoch)

        # Assert:
        self.assertTrue(timestamp.epochal)
        self.assertEqual(0, timestamp.timestamp)

    def test_can_convert_datetime_to_non_epochal_timestamp(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        timestamp = test_descriptor.network_timestamp_class.from_datetime(test_descriptor.epoch + self._get_time_delta(123))

        # Assert:
        self.assertFalse(timestamp.epochal)
        self.assertEqual(123, timestamp.timestamp)

    def test_can_convert_datetime_to_epochal_timestamp_custom_epoch(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        timestamp = test_descriptor.network_timestamp_class.from_datetime(CUSTOM_EPOCH_TIME, CUSTOM_EPOCH_TIME)

        # Assert:
        self.assertTrue(timestamp.epochal)
        self.assertEqual(0, timestamp.timestamp)

    def test_can_convert_datetime_to_non_epochal_timestamp_custom_epoch(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        # Act:
        timestamp = test_descriptor.network_timestamp_class.from_datetime(CUSTOM_EPOCH_TIME + self._get_time_delta(123), CUSTOM_EPOCH_TIME)

        # Assert:
        self.assertFalse(timestamp.epochal)
        self.assertEqual(123, timestamp.timestamp)

    # endregion

    def _get_time_delta(self, count):
        return datetime.timedelta(**{self.get_test_descriptor().time_units: count})

    @abstractmethod
    def get_test_descriptor(self):
        pass
