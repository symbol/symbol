from ..CryptoTypes import PublicKey
from .Network import Address
from .Transaction import Transaction

RECIPIENT_LENGTH = 40


class TransferTransaction(Transaction):
    """Represents a transfer transaction."""
    NAME = 'transfer'
    TYPE = 0x0101

    TYPE_HINTS = {
        'signer_public_key': PublicKey,
        'recipient_address': Address
    }

    def __init__(self, network):
        """Creates a transfer transaction for the specified network."""
        super().__init__(network, TransferTransaction.TYPE)

        self.recipient_address = None
        self.amount = 0
        self._message = None

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
        return self._message

    @message.setter
    def message(self, value):
        """Sets the message."""
        if isinstance(value, str):
            self._message = value.encode('utf8')
        else:
            self._message = bytes(value)

    def serialize_custom(self, writer):
        writer.write_int(RECIPIENT_LENGTH, 4)
        writer.write_string(str(self.recipient_address))

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

    @staticmethod
    def field_names():
        return ['recipient_address', 'amount', 'message']
