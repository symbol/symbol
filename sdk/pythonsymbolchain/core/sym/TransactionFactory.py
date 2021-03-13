from binascii import hexlify

from symbol_catbuffer.EmbeddedTransactionBuilderFactory import EmbeddedTransactionBuilderFactory
from symbol_catbuffer.NetworkTypeDto import NetworkTypeDto
from symbol_catbuffer.TransactionBuilderFactory import TransactionBuilderFactory

from ..CryptoTypes import PublicKey
from ..TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .Network import Address


class TransactionFactory:
    """Factory for creating transactions."""

    def __init__(self, network, type_parsing_rules=None):
        """Creates a factory for the specified network."""
        self.network = network
        self.type_parsing_rules = type_parsing_rules

    def _create(self, transaction_descriptor, factory_class):
        processor = TransactionDescriptorProcessor(transaction_descriptor, self.type_parsing_rules)
        processor.set_type_hints({'signerPublicKey': PublicKey})

        transaction = factory_class.createByName(
            processor.lookup_value('type'),
            processor.lookup_value('signerPublicKey'),
            NetworkTypeDto(self.network.identifier))

        processor.set_type_hints(self._build_type_hints_map(transaction))
        processor.copy_to(transaction, ['type', 'signerPublicKey'])
        return transaction

    def create(self, transaction_descriptor):
        """Creates a transaction from a transaction descriptor."""
        return self._create(transaction_descriptor, TransactionBuilderFactory)

    def create_embedded(self, transaction_descriptor):
        """Creates an embedded transaction from a transaction descriptor."""
        return self._create(transaction_descriptor, EmbeddedTransactionBuilderFactory)

    @staticmethod
    def attach_signature(transaction, signature):
        """Attaches a signature to a transaction."""
        transaction.signature = signature.bytes

        transaction_buffer = transaction.serialize()
        json_payload = '{{"payload": "{}"}}'.format(hexlify(transaction_buffer).decode('utf8').upper())
        return json_payload.encode('utf8')

    @staticmethod
    def _build_type_hints_map(transaction):
        mapping = {
            'UnresolvedAddressDto': Address,
            'KeyDto': PublicKey
        }

        type_hints = {}
        for key, hint in transaction.type_hints.items():
            if hint in mapping:
                type_hints[key] = mapping[hint]

        return type_hints
