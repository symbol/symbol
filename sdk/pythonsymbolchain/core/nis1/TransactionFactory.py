from ..BufferWriter import BufferWriter
from .TransferTransaction import TransferTransaction


class TransactionFactory:
    """Factory for creating transactions."""

    def __init__(self, network, type_parsing_rules=None):
        """Creates a factory for the specified network."""
        self.network = network
        self.type_parsing_rules = type_parsing_rules

        if not self.type_parsing_rules:
            self.type_parsing_rules = {}

    def create(self, name):
        """Creates a transaction."""
        if name not in [TransferTransaction.NAME]:
            raise ValueError('transaction named {} is not supported'.format(name))

        return TransferTransaction(self.network)

    def create_from_descriptor(self, transaction_descriptor):
        """Creates a transaction from a transaction descriptor."""
        transaction = self.create(transaction_descriptor['type'])

        for key, value in transaction_descriptor.items():
            if 'type' == key:
                continue

            if key in transaction.__dict__:
                type_hint = type(transaction).__annotations__.get(key)  # pylint: disable=no-member
                if type_hint in self.type_parsing_rules:
                    value = self.type_parsing_rules[type_hint](value)

            setattr(transaction, key, value)

        return transaction

    @staticmethod
    def attach_signature(transaction, signature):
        """Attaches a signature to a transaction."""
        writer = BufferWriter()

        transaction_buffer = transaction.serialize()
        writer.write_int(len(transaction_buffer), 4)
        writer.write_bytes(transaction_buffer)

        writer.write_int(len(signature.bytes), 4)
        writer.write_bytes(signature.bytes)
        return writer.buffer
