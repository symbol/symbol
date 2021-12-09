import argparse
import os
import sys
from pathlib import Path

import yaml
from lark import Tree

from .ast import Statement
from .AstPostProcessor import AstPostProcessor
from .CatsLarkParser import create_cats_lark_parser
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
        return self.cats_parser.type_descriptors()

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


class LarkMultiFileParser:
    """Multifile CATS parser implementation using Lark."""

    def __init__(self):
        self.parser = create_cats_lark_parser()
        self.dirname = None

        self.type_descriptors = []
        self.processed_filepaths = []

    def set_include_path(self, include_path):
        self.dirname = Path(include_path)

    def parse(self, filepath):
        if filepath in self.processed_filepaths:
            return []

        print(f'processing {filepath}...')
        self.processed_filepaths.append(filepath)

        with open(filepath, 'rt', encoding='utf8') as infile:
            contents = infile.read()

        parse_result = self.parser.parse(contents)
        if isinstance(parse_result, Statement):
            return [parse_result]

        descriptors = []
        unprocessed_trees = [child for child in parse_result.children if isinstance(child, Tree)]
        for unprocessed_tree in unprocessed_trees:
            if 'import' == unprocessed_tree.data:
                descriptors += self.parse(self.dirname / unprocessed_tree.children[0])
            else:
                raise CatsParseException(f'found unexpected unprocessed tree "{unprocessed_tree.data}"')

        # filter out trees and unattached comments
        return descriptors + [descriptor for descriptor in parse_result.children if isinstance(descriptor, Statement)]


def main():
    parser = argparse.ArgumentParser(
        prog=None if globals().get('__spec__') is None else 'python -m {}'.format(__spec__.name.partition('.')[0]),
        description='CATS code generator'
    )
    parser.add_argument('-s', '--schema', help='input CATS file', required=True)
    parser.add_argument('-i', '--include', help='schema root directory', required=True)
    parser.add_argument('-o', '--output', help='yaml output file')
    # temporary option until error checking is added to Lark implementation
    parser.add_argument('-l', '--legacy-mode', help='use legacy mode', action='store_true')
    args = parser.parse_args()

    file_parser = MultiFileParser() if args.legacy_mode else LarkMultiFileParser()
    file_parser.set_include_path(args.include)
    raw_type_descriptors = file_parser.parse(args.schema)

    try:
        if args.legacy_mode:
            type_descriptors = list(map(lambda e: {**{'name': e[0]}, **e[1]}, raw_type_descriptors.items()))
        else:
            processor = AstPostProcessor(raw_type_descriptors)
            processor.expand_named_inlines()
            type_descriptors = [model.to_legacy_descriptor() for model in processor.type_descriptors]

    except CatsParseException as ex:
        print('\033[31m{}\033[39m'.format(ex.message))
        print('\033[33m{}\033[39m'.format(ex.scope[0]))
        for location in ex.scope[1:]:
            print(' + {}'.format(location))

        sys.exit(1)

    # dump parsed type descriptors
    yaml.dump(type_descriptors, sys.stdout)

    if args.output:
        with open(args.output, 'wt', encoding='utf8') as out:
            yaml.dump(type_descriptors, out)


if '__main__' == __name__:
    main()
