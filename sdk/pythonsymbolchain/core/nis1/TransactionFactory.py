from ..BufferWriter import BufferWriter
from ..TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .ImportanceTransferTransaction import ImportanceTransferTransaction
from .TransferTransaction import TransferTransaction


class TransactionFactory:
    """Factory for creating transactions."""

    def __init__(self, network, type_parsing_rules=None):
        """Creates a factory for the specified network."""
        self.network = network
        self.type_parsing_rules = type_parsing_rules

    def _create(self, name):
        transaction_classes = [ImportanceTransferTransaction, TransferTransaction]
        if name not in [transaction_class.NAME for transaction_class in transaction_classes]:
            raise ValueError('transaction named {} is not supported'.format(name))

        return next(transaction_class for transaction_class in transaction_classes if name == transaction_class.NAME)(self.network)

    def create(self, transaction_descriptor):
        """Creates a transaction from a transaction descriptor."""
        processor = TransactionDescriptorProcessor(transaction_descriptor, self.type_parsing_rules)
        transaction = self._create(processor.lookup_value('type'))

        processor.set_type_hints(type(transaction).TYPE_HINTS)
        processor.copy_to(transaction, ['type'])
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
