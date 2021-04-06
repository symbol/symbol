from ..CryptoTypes import PublicKey
from .Transaction import Transaction


class ImportanceTransferTransaction(Transaction):
    """Represents an importance transfer transaction."""
    NAME = 'importance-transfer'
    TYPE = 0x0801

    TYPE_HINTS = {
        'signer_public_key': PublicKey,
        'remote_account_public_key': PublicKey
    }

    def __init__(self, network):
        """Creates an importance transfer transaction for the specified network."""
        super().__init__(network, ImportanceTransferTransaction.TYPE)

        self.mode = 0
        self.remote_account_public_key = None

    @property
    def fee(self):
        """Gets the (minimum) fee."""
        return 150000

    def serialize_custom(self, writer):
        writer.write_int(self.mode, 4)

        writer.write_int(PublicKey.SIZE, 4)
        writer.write_bytes(self.remote_account_public_key.bytes)

    @staticmethod
    def field_names():
        return ['mode', 'remote_account_public_key']
