import datetime

from ..NetworkTimestamp import NetworkTimestamp as BasicNetworkTimestamp
from ..NetworkTimestamp import NetworkTimestampDatetimeConverter

EPOCH_TIME = datetime.datetime(2021, 3, 16, 0, 6, 25, tzinfo=datetime.timezone.utc)


class NetworkTimestamp(BasicNetworkTimestamp):
    """Represents a symbol network timestamp with millisecond resolution."""

    def add_milliseconds(self, count):
        """Adds a specified number of milliseconds to this timestamp."""
        return NetworkTimestamp(self.timestamp + count)

    def add_seconds(self, count):
        return self.add_milliseconds(1000 * count)

    def to_datetime(self, epoch_time=EPOCH_TIME):
        """Converts this timestamp to a datetime."""
        return NetworkTimestampDatetimeConverter(epoch_time, 'milliseconds').to_datetime(self.timestamp)

    @staticmethod
    def from_datetime(reference_datetime, epoch_time=EPOCH_TIME):
        """Creates a network timestamp from a datetime."""
        return NetworkTimestamp(NetworkTimestampDatetimeConverter(epoch_time, 'milliseconds').to_difference(reference_datetime))
