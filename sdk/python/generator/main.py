#!/usr/bin/python

import argparse
import shutil
from pathlib import Path

import yaml
from AbstractImplMap import AbstractImplMap
from EnumTypeFormatter import EnumTypeFormatter
from FactoryFormatter import FactoryClassFormatter, FactoryFormatter
from PodTypeFormatter import PodTypeFormatter
from printers import BuiltinPrinter, create_pod_printer
from StructTypeFormatter import StructFormatter
from type_objects import ArrayObject, EnumObject, IntObject, StructObject
from TypeFormatter import TypeFormatter


def fix_name(field_name):
    if field_name in ['size', 'type']:
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

    def get_type(self):
        return self.type_instance

    def get_printer(self):
        return self.printer

    def set_bound_field(self, other_field):
        self.bound_field = other_field

    def is_bound(self):
        return self.bound_field is not None

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


def find_linked_abstract_type(type_map, struct_type: StructObject):
    for field_yaml_descriptor in struct_type.yaml_descriptor['layout']:
        type_instance = type_map.get(field_yaml_descriptor['type'], None)
        field_not_inlined = 'inline' != field_yaml_descriptor.get('disposition', None)

        if field_not_inlined and type_instance and type_instance.is_struct and type_instance.is_abstract:
            return type_instance

    return None


def process_struct(type_map, struct_type: StructObject, abstract_impl_map: AbstractImplMap):
    for field_yaml_descriptor in struct_type.yaml_descriptor['layout']:
        fix_item(field_yaml_descriptor, 'size')
        type_instance = type_map.get(field_yaml_descriptor['type'], None)

        processed = False
        if 'disposition' in field_yaml_descriptor:
            if 'inline' == field_yaml_descriptor['disposition']:
                # find actual (existing) struct in type_map and copy fields
                inlined_type_instance = type_map[field_yaml_descriptor['type']]
                for inlined_field in inlined_type_instance.get_layout():
                    struct_type.add_field(inlined_field)

                struct_type.has_inlines = True

                if inlined_type_instance.is_abstract:
                    abstract_impl_map.add(inlined_type_instance, struct_type)

                processed = True

            # array, array sized, array fill
            elif field_yaml_descriptor['disposition'].startswith('array'):
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

    # go through structs and bind size fields to arrays
    for field in struct_type.get_layout():
        if field.type_instance.is_array and isinstance(field.get_type().get_size(), str):
            size_name = field.get_type().get_size()
            # 'unfix' name when searching for associated 'size' field:
            #  * fields are added using 'original' name,
            #  * but 'size' property of given array type is already fixed
            #
            # (currently this only happens for metadata value)
            if 'size_' == size_name:
                size_name = 'size'

            size_field = struct_type.get_field_by_name(size_name)
            size_field.set_bound_field(field)


def generate_files(yaml_descriptors, output_directory: Path):
    # build map of types
    type_map = {}
    abstract_impl_map = AbstractImplMap()

    types = []
    for yaml_descriptor in yaml_descriptors:
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

    array_helpers = Path('ArrayHelpers.py')
    shutil.copy(str(array_helpers), str(output_directory / array_helpers))

    with open(output_directory / '__init__.py', 'w', encoding='utf8') as output_file:
        output_file.write(
            '''#!/usr/bin/python
#
# Code generated by catbuffer python generator; DO NOT EDIT.

from __future__ import annotations

from binascii import hexlify
from enum import Enum, Flag
from typing import ByteString, List

from .ArrayHelpers import ArrayHelpers, BaseValue, FixedByteArray


'''
        )
        for idx, type_instance in enumerate(types):
            if type_instance.is_struct:
                abstract_type = find_linked_abstract_type(type_map, type_instance)
                if abstract_type:
                    factory_generator = FactoryClassFormatter(FactoryFormatter(abstract_impl_map, abstract_type))
                    output_file.write(str(factory_generator))
                    output_file.write('\n\n')

                    # hack: hardcode non-embedded transaction factory
                    # because right now nothing is referencing it

                    abstract_type = type_map.get('Transaction', None)
                    factory_generator = FactoryClassFormatter(FactoryFormatter(abstract_impl_map, abstract_type))
                    output_file.write(str(factory_generator))
                    output_file.write('\n\n')

            generator = TypeFormatter(to_type_formatter_instance(type_instance))

            output_file.write(str(generator))
            if idx != len(types) - 1:
                output_file.write('\n\n')


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
