from binascii import hexlify

from symbolchain import sc

from ..ByteArray import ByteArray
from ..CryptoTypes import Hash256, PublicKey, Signature
from ..TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .ExtendedTypeParsingRules import build_type_hints_map, extend_type_parsing_rules
from .IdGenerator import generate_mosaic_id, generate_namespace_id
from .Network import Address


def symbol_type_converter(value):
    if isinstance(value, Address):
        return sc.UnresolvedAddress(value.bytes)

    if isinstance(value, PublicKey):
        return sc.PublicKey(value.bytes)

    if isinstance(value, Hash256):
        return sc.Hash256(value.bytes)

    if isinstance(value, Signature):
        return sc.Signature(value.bytes)

    if isinstance(value, ByteArray):
        raise ValueError('ByteArray not allowed ')

    return value


class TransactionFactory:
    """Factory for creating transactions."""

    def __init__(self, network, type_parsing_rules=None):
        """Creates a factory for the specified network."""
        self.network = network
        self.type_parsing_rules = type_parsing_rules

    def _create(self, transaction_descriptor, factory_class):
        extended_type_parsing_rules = extend_type_parsing_rules(symbol_type_converter, self.type_parsing_rules)
        processor = TransactionDescriptorProcessor(transaction_descriptor, extended_type_parsing_rules, symbol_type_converter)

        processor.set_type_hints({'signer_public_key': PublicKey})

        transaction_type = processor.lookup_value('type')
        transaction = factory_class.create_by_name(transaction_type)
        transaction.signer_public_key = processor.lookup_value('signer_public_key')
        transaction.network = sc.NetworkType(self.network.identifier)

        all_type_hints = build_type_hints_map(transaction)
        processor.set_type_hints(all_type_hints)
        processor.copy_to(transaction, ['type', 'signer_public_key'])

        # autogenerate artifact ids
        if 'namespace_registration_transaction' == transaction_type:
            transaction.id = sc.NamespaceId(generate_namespace_id(transaction.name, transaction.parent_id.value))
        elif 'mosaic_definition_transaction' == transaction_type:
            address = self.network.public_key_to_address(PublicKey(transaction.signer_public_key.bytes))
            transaction.id = sc.MosaicId(generate_mosaic_id(address, transaction.nonce.value))

        # auto encode strings
        # _todo: maybe instead of doing this, there should be hint on: `namespaceRegistration.name` and `transfer.message`
        for key in transaction_descriptor.keys():
            if 'type' == key:
                key = 'type_'

            if key in all_type_hints:
                continue

            value = getattr(transaction, key)
            if isinstance(value, str):
                setattr(transaction, key, value.encode('utf8'))

        return transaction

    def create(self, transaction_descriptor):
        """Creates a transaction from a transaction descriptor."""
        transaction_descriptor['type'] += '_transaction'
        return self._create(transaction_descriptor, sc.TransactionFactory)

    def create_embedded(self, transaction_descriptor):
        """Creates an embedded transaction from a transaction descriptor."""
        transaction_descriptor['type'] += '_transaction'
        return self._create(transaction_descriptor, sc.EmbeddedTransactionFactory)

    @staticmethod
    def attach_signature(transaction, signature):
        """Attaches a signature to a transaction."""
        transaction.signature = sc.Signature(signature.bytes)
        transaction_buffer = transaction.serialize()
        hex_payload = hexlify(transaction_buffer).decode('utf8').upper()
        json_payload = f'{{"payload": "{hex_payload}"}}'
        return json_payload.encode('utf8')
