from binascii import hexlify

from ..BufferWriter import BufferWriter
from ..CryptoTypes import PublicKey
from .Network import Address
from .NetworkTimestamp import NetworkTimestamp

RECIPIENT_LENGTH = 40
SIGNER_LENGTH = 32


class TransferTransaction:
    """Represents a transfer transaction."""
    NAME = 'transfer'
    TYPE = 0x0101

    signer: PublicKey
    recipient: Address

    def __init__(self, network):
        """Creates a transfer transaction for the specified network."""
        self.type = TransferTransaction.TYPE
        self.version = (network.identifier << 24) + 1
        self.timestamp = 0
        self.signer = None

        self.recipient = None
        self.amount = 0
        self.__message = None

    @property
    def deadline(self):
        """Gets the deadline."""
        return NetworkTimestamp(self.timestamp).add_hours(1).timestamp

    @property
    def fee(self):
        """Gets the (minimum) fee."""
        fee = min(25, max(1, self.amount // 10000000000))

        if self.message:
            fee += len(self.message) // 32 + 1

        return int(0.05 * fee * 1000000)

    @property
    def message(self):
        """Gets the message."""
        return self.__message

    @message.setter
    def message(self, value):
        """Sets the message."""
        if isinstance(value, str):
            self.__message = value.encode('utf8')
        else:
            self.__message = bytes(value)

    def serialize(self):
        """Serializes this transfer to a buffer."""
        writer = BufferWriter()

        writer.write_int(self.type, 4)
        writer.write_int(self.version, 4)
        writer.write_int(self.timestamp, 4)

        writer.write_int(SIGNER_LENGTH, 4)
        writer.write_bytes(self.signer.bytes)

        writer.write_int(self.fee, 8)
        writer.write_int(self.deadline, 4)

        writer.write_int(RECIPIENT_LENGTH, 4)
        writer.write_string(str(self.recipient))

        writer.write_int(self.amount, 8)

        if not self.message:
            writer.write_int(0, 4)
        else:
            message_length = len(self.message)

            writer.write_int(message_length + 8, 4)
            writer.write_int(1, 4)
            writer.write_int(message_length, 4)
            writer.write_bytes(self.message)

        return writer.buffer

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
        formatter = TransferTransaction._FieldFormatter()
        for name in ['type', 'version', 'timestamp', 'signer', 'fee', 'deadline', 'recipient', 'amount', 'message']:
            formatter.add_field(name, getattr(self, name))

        return str(formatter)
