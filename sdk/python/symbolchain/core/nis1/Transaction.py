from abc import abstractmethod
from binascii import hexlify

from ..BufferWriter import BufferWriter
from ..CryptoTypes import PublicKey
from .NetworkTimestamp import NetworkTimestamp


class Transaction:
    """Represents a transaction."""

    def __init__(self, network, transaction_type):
        """Creates a transaction for the specified network."""
        self.type = transaction_type
        self.version = (network.identifier << 24) + 1
        self.signer_public_key = None
        self.deadline = 0

    @property
    def timestamp(self):
        """Gets the timestamp."""
        return max(0, NetworkTimestamp(self.deadline).add_hours(-24).timestamp)

    @property
    def fee(self):
        """Gets the (minimum) fee."""

    def serialize(self):
        """Serializes this transaction to a buffer."""
        writer = BufferWriter()

        writer.write_int(self.type, 4)
        writer.write_int(self.version, 4)
        writer.write_int(self.timestamp, 4)

        writer.write_int(PublicKey.SIZE, 4)
        writer.write_bytes(self.signer_public_key.bytes)

        writer.write_int(self.fee, 8)
        writer.write_int(self.deadline, 4)

        self.serialize_custom(writer)
        return writer.buffer

    @abstractmethod
    def serialize_custom(self, writer):
        """Serializes custom transaction fields to a writer."""

    @abstractmethod
    def field_names(self):
        """Gets names of custom transaction fields to include in the string representation."""

    class _FieldFormatter:
        def __init__(self):
            self.field_pairs = []

        def add_field(self, key, value):
            self.field_pairs.append((key, self._format_value(value)))

        @staticmethod
        def _format_value(value):
            if isinstance(value, int):
                return '{0} [0x{0:X}]'.format(value)

            if isinstance(value, bytes):
                try:
                    return value.decode('utf8')
                except UnicodeError:
                    return hexlify(value).upper().decode('utf8')

            return value

        def __str__(self):
            max_name_length = max(len(pair[0]) for pair in self.field_pairs)
            formatted_fields = ['{} = {}'.format(pair[0].rjust(max_name_length, ' '), pair[1]) for pair in self.field_pairs]
            return '\n'.join(formatted_fields)

    def __str__(self):
        formatter = Transaction._FieldFormatter()
        for name in ['type', 'version', 'timestamp', 'signer_public_key', 'fee', 'deadline'] + self.field_names():
            formatter.add_field(name, getattr(self, name))

        return str(formatter)
