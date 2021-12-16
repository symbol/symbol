import argparse
import sys
from pathlib import Path

import yaml
from lark import Tree

from .ast import AstException, Statement
from .AstPostProcessor import AstPostProcessor
from .AstValidator import AstValidator
from .CatsLarkParser import create_cats_lark_parser


def print_error(message):
    print(f'\033[31m{message}\033[39m')


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

        print(f'processing \033[33m{filepath}\033[39m...')
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
                raise AstException(f'found unexpected unprocessed tree "{unprocessed_tree.data}"')

        # filter out trees and unattached comments
        return descriptors + [descriptor for descriptor in parse_result.children if isinstance(descriptor, Statement)]


def validate(raw_type_descriptors, stage, mode):
    validator = AstValidator(raw_type_descriptors)
    validator.set_validation_mode(mode)
    validator.validate()
    if validator.errors:
        print_error(f'[ERRORS DETECTED AT STAGE {stage}]')
        for error in validator.errors:
            print(f' + {error}')

        sys.exit(2)


def main():
    parser = argparse.ArgumentParser(
        prog=None if globals().get('__spec__') is None else f'python -m {__spec__.name.partition(".")[0]}',
        description='CATS code generator'
    )
    parser.add_argument('-s', '--schema', help='input CATS file', required=True)
    parser.add_argument('-i', '--include', help='schema root directory', required=True)
    parser.add_argument('-o', '--output', help='yaml output file')
    args = parser.parse_args()

    file_parser = LarkMultiFileParser()
    file_parser.set_include_path(args.include)

    try:
        raw_type_descriptors = file_parser.parse(args.schema)
    except (AstException, OSError) as ex:
        print_error(str(ex))
        sys.exit(1)

    processor = AstPostProcessor(raw_type_descriptors)

    validate(raw_type_descriptors, 'PRE EXPANSION', AstValidator.Mode.PRE_EXPANSION)

    processor.apply_attributes()
    processor.expand_named_inlines()
    processor.expand_unnamed_inlines()

    validate(raw_type_descriptors, 'POST EXPANSION', AstValidator.Mode.POST_EXPANSION)

    type_descriptors = [model.to_legacy_descriptor() for model in processor.type_descriptors]

    # dump parsed type descriptors
    yaml.dump(type_descriptors, sys.stdout)

    if args.output:
        with open(args.output, 'wt', encoding='utf8') as out:
            yaml.dump(type_descriptors, out)


if '__main__' == __name__:
    main()
