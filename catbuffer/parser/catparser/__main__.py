import argparse
import os
import sys

import yaml

from .CatsParseException import CatsParseException
from .CatsParser import CatsParser


class MultiFileParser:
    """CATS parser that resolves imports in global namespace"""
    def __init__(self):
        self.cats_parser = CatsParser(self._process_import_file)
        self.dirname = None
        self.imported_filenames = []

    def set_include_path(self, include_path):
        self.dirname = include_path

    def parse(self, schema_filename):
        self._process_file(schema_filename)

    def _process_import_file(self, filename):
        if filename in self.imported_filenames:
            return

        self.imported_filenames.append(filename)
        filename = os.path.join(self.dirname, filename)
        self._process_file(filename)

    def _process_file(self, filename):
        self.cats_parser.push_scope(filename)

        with open(filename) as input_file:
            lines = input_file.readlines()
            for line in lines:
                self.cats_parser.process_line(line)

        self.cats_parser.commit()
        self.cats_parser.pop_scope()


def _dump_type_descriptors(type_descriptors, out):
    needs_separator = False
    for key in type_descriptors:
        if needs_separator:
            out.write('\n')

        out.write('name: {}\n'.format(key))
        yaml.dump(type_descriptors[key], out)
        needs_separator = True


def main():
    parser = argparse.ArgumentParser(
        prog=None if globals().get('__spec__') is None else 'python -m {}'.format(__spec__.name.partition('.')[0]),
        description='CATS code generator'
    )
    parser.add_argument('-s', '--schema', help='input CATS file', required=True)
    parser.add_argument('-i', '--include', help='schema root directory', required=True)
    parser.add_argument('-o', '--output', help='yaml output file')
    args = parser.parse_args()

    file_parser = MultiFileParser()
    file_parser.set_include_path(args.include)

    try:
        file_parser.parse(args.schema)
        type_descriptors = file_parser.cats_parser.type_descriptors()
    except CatsParseException as ex:
        print('\033[31m{}\033[39m'.format(ex.message))
        print('\033[33m{}\033[39m'.format(ex.scope[0]))
        for location in ex.scope[1:]:
            print(' + {}'.format(location))

        sys.exit(1)

    # dump parsed type descriptors
    _dump_type_descriptors(type_descriptors, sys.stdout)

    if args.output:
        with open(args.output, 'wt') as out:
            _dump_type_descriptors(type_descriptors, out)


if '__main__' == __name__:
    main()
