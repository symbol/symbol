#!/usr/bin/python

import argparse
from pathlib import Path

import yaml

from .AbstractImplMap import AbstractImplMap
from .EnumTypeFormatter import EnumTypeFormatter
from .FactoryFormatter import FactoryClassFormatter, FactoryFormatter
from .PodTypeFormatter import PodTypeFormatter
from .printers import BuiltinPrinter, create_pod_printer
from .StructTypeFormatter import StructFormatter
from .type_objects import ArrayObject, EnumObject, IntObject, StructObject
from .TypeFormatter import TypeFormatter


def fix_name(field_name):
    if field_name in ['size', 'type', 'property']:
        return f'{field_name}_'
    return field_name


def fix_item(yaml_descriptor, name):
    if name not in yaml_descriptor:
        return

    yaml_descriptor[name] = fix_name(yaml_descriptor[name])


class Field:
    def __init__(self, yaml_descriptor, field_name, type_instance, printer):
        self.yaml_descriptor = yaml_descriptor
        self.original_field_name = field_name
        self.type_instance = type_instance
        self.printer = printer
        self.bound_field = None
        self.size_fields = []

    def get_type(self):
        return self.type_instance

    def get_printer(self):
        return self.printer

    def set_bound_field(self, other_field):
        self.bound_field = other_field

    def is_bound(self):
        return self.bound_field is not None

    def add_sizeof_field(self, other_field):
        self.size_fields.append(other_field)

    def is_const(self):
        return 'disposition' in self.yaml_descriptor and 'const' == self.yaml_descriptor['disposition']

    def is_conditional(self):
        return 'condition' in self.yaml_descriptor


def to_type_formatter_instance(type_instance):
    mapping = {
        'int': PodTypeFormatter,
        'array': PodTypeFormatter,
        'enum': EnumTypeFormatter,
        'struct': StructFormatter,
    }

    type_formatter = mapping[type_instance.base_typename]
    return type_formatter(type_instance)


def to_virtual_type_instance(yaml_descriptor):
    object_type = None
    if yaml_descriptor['type'] == 'struct':
        object_type = StructObject
    elif yaml_descriptor['type'] == 'enum':
        object_type = EnumObject
    elif yaml_descriptor['type'] == 'byte':
        size = yaml_descriptor['size']
        # can be either 'bound' array or normal
        if isinstance(size, str):
            object_type = ArrayObject
        else:
            object_type = ArrayObject if size > 8 else IntObject

    return object_type(yaml_descriptor)


def create_printer(type_instance, name):
    if type_instance.is_builtin:
        return BuiltinPrinter(type_instance, name)

    return create_pod_printer(type_instance, name)


def process_concrete_struct(type_map, struct_type, abstract_impl_map):
    factory_type = type_map[struct_type.yaml_descriptor['factory_type']]

    discriminators = factory_type.yaml_descriptor.get('discriminator', [])
    for i, discriminator in enumerate(discriminators):
        discriminators[i] = fix_name(discriminator)

    for init in factory_type.yaml_descriptor['initializers']:
        fix_item(init, 'target_property_name')

    abstract_impl_map.add(factory_type, struct_type)


def bind_size_fields(struct_type):
    # go through structs and bind size fields to arrays
    for field in struct_type.layout:
        if field.type_instance.is_array and isinstance(field.get_type().size, str):
            size_field_name = field.get_type().size
            # 'unfix' name when searching for associated 'size' field:
            #  * fields are added using 'original' name,
            #  * but 'size' property of given array type is already fixed
            #
            # (currently this only happens for metadata value)
            if 'size_' == size_field_name:
                size_field_name = 'size'

            size_field = struct_type.get_field_by_name(size_field_name)
            size_field.set_bound_field(field)

        if field.type_instance.sizeof_value:
            struct_field = struct_type.get_field_by_name(field.yaml_descriptor['value'])
            field.set_bound_field(struct_field)

            struct_field.add_sizeof_field(field)


def process_struct(type_map, struct_type: StructObject, abstract_impl_map: AbstractImplMap):
    if 'factory_type' in struct_type.yaml_descriptor:
        process_concrete_struct(type_map, struct_type, abstract_impl_map)

    for field_yaml_descriptor in struct_type.yaml_descriptor['layout']:
        fix_item(field_yaml_descriptor, 'size')
        type_instance = type_map.get(field_yaml_descriptor['type'], None)

        processed = False
        if 'disposition' in field_yaml_descriptor:
            # array, array sized, array fill
            if field_yaml_descriptor['disposition'].startswith('array'):
                element_type = type_instance
                type_instance = ArrayObject(field_yaml_descriptor)
                type_instance.element_type = element_type
                type_instance.is_builtin = False

        if not type_instance:
            type_instance = to_virtual_type_instance(field_yaml_descriptor)
            type_instance.is_builtin = False

        if not processed:
            field_printer = create_printer(type_instance, fix_name(field_yaml_descriptor['name']))
            struct_type.add_field(
                Field(
                    field_yaml_descriptor,
                    field_yaml_descriptor['name'],
                    type_instance,
                    field_printer,
                )
            )

    bind_size_fields(struct_type)


def generate_files(yaml_descriptors, output_directory: Path):
    # build map of types
    type_map = {}
    abstract_impl_map = AbstractImplMap()

    types = []
    for yaml_descriptor in yaml_descriptors:
        # 'size' entry contains name of a field that holds/tracks entity size
        if 'struct' == yaml_descriptor['type']:
            fix_item(yaml_descriptor, 'size')

        instance = to_virtual_type_instance(yaml_descriptor)
        types.append(instance)
        type_map[yaml_descriptor['name']] = instance

    # process struct fields
    for type_instance in types:
        if not type_instance.is_struct:
            continue

        process_struct(type_map, type_instance, abstract_impl_map)

    output_directory.mkdir(exist_ok=True)

    with open(output_directory / '__init__.py', 'w', encoding='utf8') as output_file:
        output_file.write(
            '''#!/usr/bin/python
#
# Code generated by catbuffer python generator; DO NOT EDIT.

from __future__ import annotations

from binascii import hexlify
from enum import Enum, Flag
from typing import ByteString, List, TypeVar

from ..core.ArrayHelpers import ArrayHelpers
from ..core.BaseValue import BaseValue
from ..core.ByteArray import ByteArray


# string or bytes
StrBytes = TypeVar('StrBytes', str, bytes)

'''
        )
        for type_instance in types:
            generator = TypeFormatter(to_type_formatter_instance(type_instance))
            output_file.write(str(generator))
            output_file.write('\n\n')

        factories = []
        for type_instance in types:
            if not (type_instance.is_struct and type_instance.is_abstract):
                continue

            factory_generator = FactoryClassFormatter(FactoryFormatter(abstract_impl_map, type_instance))
            factories.append(str(factory_generator))

        output_file.write('\n\n'.join(factories))


def main():
    parser = argparse.ArgumentParser(
        prog=None
        if globals().get('__spec__') is None
        else f'python -m {__spec__.name.partition(".")[0]}',
        description='catbuffer python generator',
    )
    parser.add_argument('-i', '--input', help='yaml input file', required=True)
    parser.add_argument('-o', '--output', help='output directory', default='symbol_catbuffer')
    args = parser.parse_args()

    with open(args.input, 'r', encoding='utf8') as input_file:
        type_descriptors = yaml.safe_load(input_file)
        generate_files(type_descriptors, Path(args.output))


if '__main__' == __name__:
    main()
