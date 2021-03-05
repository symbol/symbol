import datetime

from ..NetworkTimestamp import NetworkTimestamp as BasicNetworkTimestamp
from ..NetworkTimestamp import NetworkTimestampDatetimeConverter

EPOCH_TIME = datetime.datetime(2015, 3, 29, 0, 6, 25, tzinfo=datetime.timezone.utc)


class NetworkTimestamp(BasicNetworkTimestamp):
    """Represents a nis network timestamp with second resolution."""

    def add_seconds(self, count):
        return NetworkTimestamp(self.timestamp + count)

    def to_datetime(self, epoch_time=EPOCH_TIME):
        """Converts this timestamp to a datetime."""
        return NetworkTimestampDatetimeConverter(epoch_time, 'seconds').to_datetime(self.timestamp)

    @staticmethod
    def from_datetime(reference_datetime, epoch_time=EPOCH_TIME):
        """Creates a network timestamp from a datetime."""
        return NetworkTimestamp(NetworkTimestampDatetimeConverter(epoch_time, 'seconds').to_difference(reference_datetime))
