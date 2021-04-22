import importlib
from binascii import hexlify

from symbol_catbuffer.EmbeddedTransactionBuilderFactory import EmbeddedTransactionBuilderFactory
from symbol_catbuffer.NetworkTypeDto import NetworkTypeDto
from symbol_catbuffer.TransactionBuilderFactory import TransactionBuilderFactory

from ..ByteArray import ByteArray
from ..CryptoTypes import PublicKey
from ..TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .IdGenerator import generate_mosaic_id, generate_namespace_id
from .Network import Address


def get_catbuffer_class(name):
    return getattr(importlib.import_module('symbol_catbuffer.{}'.format(name)), name)


def name_to_enum_value(mapping, enum_type, enum_value_name):
    if enum_value_name not in mapping:
        raise RuntimeError('unknown value {} for type {}'.format(enum_value_name, enum_type))
    return mapping[enum_value_name]


def create_flags_parser(name):
    string_to_enum = dict(map(lambda key: (key.name.lower(), key), get_catbuffer_class(name)))

    def parser(flags):
        if not isinstance(flags, str):
            return flags

        enum_array = list(map(lambda flag_name: name_to_enum_value(string_to_enum, name, flag_name), flags.split(' ')))
        return enum_array

    return parser


def create_array_parser(name):
    string_to_enum = dict(map(lambda key: (key.name.lower(), key), get_catbuffer_class(name)))

    def parser(flags):
        if not isinstance(flags, list) or not isinstance(flags[0], str):
            return flags

        enum_array = list(map(lambda flag_name: name_to_enum_value(string_to_enum, name, flag_name), flags))
        return enum_array

    return parser


def create_enum_parser(name):
    string_to_enum = dict(map(lambda key: (key.name.lower(), key), get_catbuffer_class(name)))

    def parser(enum_value):
        if not isinstance(enum_value, str):
            return enum_value

        return name_to_enum_value(string_to_enum, name, enum_value)

    return parser


def sym_type_converter(value):
    if isinstance(value, ByteArray):
        return value.bytes

    return value


class TransactionFactory:
    """Factory for creating transactions."""

    def __init__(self, network, type_parsing_rules=None):
        """Creates a factory for the specified network."""
        self.network = network
        self.type_parsing_rules = type_parsing_rules

    def _extend_type_parsing_rules(self):
        default_rules = {
            'MosaicFlagsDto': create_flags_parser('MosaicFlagsDto'),
            'AccountRestrictionFlagsDto': create_flags_parser('AccountRestrictionFlagsDto'),

            'array_EntityTypeDto': create_array_parser('EntityTypeDto'),

            'AliasActionDto': create_enum_parser('AliasActionDto'),
            'LinkActionDto': create_enum_parser('LinkActionDto'),
            'LockHashAlgorithmDto': create_enum_parser('LockHashAlgorithmDto'),
            'MosaicRestrictionTypeDto': create_enum_parser('MosaicRestrictionTypeDto'),
            'MosaicSupplyChangeActionDto': create_enum_parser('MosaicSupplyChangeActionDto'),
            'NamespaceRegistrationTypeDto': create_enum_parser('NamespaceRegistrationTypeDto')
        }

        if not self.type_parsing_rules:
            return default_rules

        return {**default_rules, **self.type_parsing_rules}

    def _create(self, transaction_descriptor, factory_class):
        processor = TransactionDescriptorProcessor(transaction_descriptor, self._extend_type_parsing_rules(), sym_type_converter)

        processor.set_type_hints({'signer_public_key': PublicKey})

        transaction_type = processor.lookup_value('type')

        transaction = factory_class.create_by_name(
            transaction_type,
            processor.lookup_value('signer_public_key'),
            NetworkTypeDto(self.network.identifier))

        all_type_hints = self._build_type_hints_map(transaction)
        processor.set_type_hints(all_type_hints)
        processor.copy_to(transaction, ['type', 'signer_public_key'])

        # autogenerate artifact ids
        if 'namespaceRegistration' == transaction_type:
            transaction.id = generate_namespace_id(transaction.name, transaction.parent_id)
        elif 'mosaicDefinition' == transaction_type:
            address = self.network.public_key_to_address(PublicKey(transaction.signer_public_key))
            transaction.id = generate_mosaic_id(address, transaction.nonce)

        # auto encode strings
        for key in transaction_descriptor.keys():
            if key in all_type_hints:
                continue

            value = getattr(transaction, key)
            if isinstance(value, str):
                setattr(transaction, key, value.encode('utf8'))

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
            rule_name = None
            if hint.startswith('array[enum:'):
                rule_name = 'array_' + hint[len('array[enum:'):-1]
            elif hint.startswith('enum:') and hint not in ('enum:NetworkTypeDto', 'enum:EntityTypeDto'):
                rule_name = hint[len('enum:'):]
            elif hint.endswith('FlagsDto'):
                rule_name = hint
            else:
                rule_name = mapping.get(hint, None)

            if rule_name:
                type_hints[key] = rule_name

        return type_hints
