import functools
import importlib
import inspect
import operator

from symbolchain.sc import BaseValue

from ..CryptoTypes import Hash256
from ..TransactionDescriptorProcessor import TransactionDescriptorProcessor
from .Network import Address


def _get_catbuffer_class(name):
    return getattr(importlib.import_module('symbolchain.sc'), name)


def _name_to_enum_value(mapping, enum_type, enum_value_name):
    if enum_value_name not in mapping:
        raise RuntimeError(f'unknown value {enum_value_name} for type {enum_type}')
    return mapping[enum_value_name]


def _create_flags_parser(name):
    cls = _get_catbuffer_class(name)
    string_to_enum = dict(map(lambda key: (key.name.lower(), key), cls))

    def parser(flags):
        if isinstance(flags, str):
            enum_array = list(map(lambda flag_name: _name_to_enum_value(string_to_enum, name, flag_name), flags.split(' ')))
            return functools.reduce(operator.or_, enum_array)

        if isinstance(flags, int):
            return cls(flags)

        return flags

    return parser


def _create_object_array_parser(name, existing_rules):
    element_rule = existing_rules[name]

    def parser(values):
        return list(map(element_rule, values))

    return parser


def _create_enum_parser(name):
    cls = _get_catbuffer_class(name)
    string_to_enum = dict(map(lambda key: (key.name.lower(), key), cls))

    def parser(enum_value):
        if isinstance(enum_value, str):
            return _name_to_enum_value(string_to_enum, name, enum_value)

        if isinstance(enum_value, int):
            return cls(enum_value)

        return enum_value

    return parser


def _wrap_sdk_type(type_name):
    return lambda e: type_name(e) if not isinstance(e, type_name) else e


def build_type_hints_map(transaction):
    type_hints = {}
    for key, hint in transaction.TYPE_HINTS.items():
        rule_name = None
        if hint.startswith('array['):
            rule_name = hint
        elif hint.startswith('enum:'):
            rule_name = hint[len('enum:'):]
        elif hint.startswith('pod:'):
            rule_name = hint[len('pod:'):]
        elif hint.startswith('struct:'):
            rule_name = hint

        if rule_name:
            type_hints[key] = rule_name

    return type_hints


def _create_struct_parser(struct_name, symbol_type_converter, base_type_parsing_rules):
    struct_class = _get_catbuffer_class(struct_name)

    def parser(struct_descriptor):
        extended_type_parsing_rules = extend_type_parsing_rules(symbol_type_converter, base_type_parsing_rules)
        struct_processor = TransactionDescriptorProcessor(struct_descriptor, extended_type_parsing_rules, symbol_type_converter)
        struct_obj = struct_class()

        all_type_hints = build_type_hints_map(struct_obj)
        struct_processor.set_type_hints(all_type_hints)

        struct_processor.copy_to(struct_obj)
        return struct_obj

    return parser


def extend_type_parsing_rules(symbol_type_converter, base_type_parsing_rules=None):
    # note, some of those below could be automatically generated,
    # but right now symbol.yml contains also state-related objects
    flag_parsers = {
        name: _create_flags_parser(name) for name in ['MosaicFlags', 'AccountRestrictionFlags']
    }
    enum_parser = {
        name: _create_enum_parser(name) for name in [
            'TransactionType', 'AliasAction', 'LinkAction', 'LockHashAlgorithm',
            'MosaicRestrictionType', 'MosaicSupplyChangeAction', 'NamespaceRegistrationType']
    }
    other_rules = {
        'UnresolvedAddress': _wrap_sdk_type(Address),
        'Address': _wrap_sdk_type(Address),
        'Hash256': _wrap_sdk_type(Hash256),

        'struct:UnresolvedMosaic': _create_struct_parser('UnresolvedMosaic', symbol_type_converter, base_type_parsing_rules),
    }
    default_rules = {**flag_parsers, **enum_parser, **other_rules}

    catbuffer_module = importlib.import_module('symbolchain.sc')
    for class_name in dir(catbuffer_module):
        cls = getattr(catbuffer_module, class_name)
        if inspect.isclass(cls):
            if issubclass(cls, BaseValue) and cls != BaseValue:
                default_rules[class_name] = cls

    final_rules = default_rules if not base_type_parsing_rules else {**default_rules, **base_type_parsing_rules}

    final_rules['array[UnresolvedMosaicId]'] = _create_object_array_parser('UnresolvedMosaicId', final_rules)
    final_rules['array[TransactionType]'] = _create_object_array_parser('TransactionType', final_rules)
    final_rules['array[UnresolvedAddress]'] = _create_object_array_parser('UnresolvedAddress', final_rules)

    # struct: prefix should be on both or none of those
    final_rules['array[UnresolvedMosaic]'] = _create_object_array_parser('struct:UnresolvedMosaic', final_rules)

    return final_rules
