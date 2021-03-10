from binascii import hexlify

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

    def _create(self, name, signer_public_key):
        return TransactionBuilderFactory.createByName(name, signer_public_key, NetworkTypeDto(self.network.identifier))

    def create(self, transaction_descriptor):
        """Creates a transaction from a transaction descriptor."""
        processor = TransactionDescriptorProcessor(transaction_descriptor, self.type_parsing_rules)
        processor.set_type_hints({'signerPublicKey': PublicKey})
        transaction = self._create(processor.lookup_value('type'), processor.lookup_value('signerPublicKey'))

        processor.set_type_hints(self._build_type_hints_map(transaction))
        processor.copy_to(transaction, ['type', 'signerPublicKey'])
        return transaction

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
