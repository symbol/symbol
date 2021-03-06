import datetime
from abc import abstractmethod


class NetworkTimestamp:
    """Represents a network timestamp."""

    def __init__(self, timestamp):
        """Creates a timestamp."""
        self.timestamp = timestamp

    @property
    def epochal(self):
        """Determines if this is the epochal timestamp."""
        return 0 == self.timestamp

    @abstractmethod
    def add_seconds(self, count):
        """Adds a specified number of seconds to this timestamp."""

    def add_minutes(self, count):
        """Adds a specified number of minutes to this timestamp."""
        return self.add_seconds(60 * count)

    def add_hours(self, count):
        """Adds a specified number of hours to this timestamp."""
        return self.add_minutes(60 * count)

    def __eq__(self, other):
        return isinstance(other, NetworkTimestamp) and self.timestamp == other.timestamp

    def __str__(self):
        return str(self.timestamp)


class NetworkTimestampDatetimeConverter:
    """Provides utilities for converting between network timestamps and datetimes."""

    def __init__(self, epoch, time_units):
        """Creates a converter given an epoch and base time units."""
        self.epoch = epoch
        self.time_units = time_units

    def to_datetime(self, raw_timestamp):
        """Converts a network timestamp to a datetime."""
        return self.epoch + datetime.timedelta(**{self.time_units: raw_timestamp})

    def to_difference(self, reference_datetime):
        """Creates a network timestamp from a datetime."""
        if reference_datetime < self.epoch:
            raise ValueError('timestamp cannot be before epoch')

        return (reference_datetime - self.epoch) / datetime.timedelta(**{self.time_units: 1})
