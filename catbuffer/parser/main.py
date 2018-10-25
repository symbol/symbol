# pylint: disable=too-few-public-methods
import argparse
import os
import pprint
from catparser.CatsParser import CatsParser


class MultiFileParser:
    """CATS parser that resolves imports in global namespace"""
    def __init__(self):
        self.cats_parser = CatsParser(self._process_import_file)
        self.dirname = None

    def parse(self, filename):
        self.dirname = os.path.dirname(filename)
        self._process_file(filename)

    def _process_import_file(self, filename):
        filename = os.path.join(self.dirname, filename)
        self._process_file(filename)

    def _process_file(self, filename):
        self.cats_parser.push_scope(filename)

        with open(filename) as input_file:
            lines = input_file.readlines()
            for line in lines:
                self.cats_parser.process_line(line)

        self.cats_parser.pop_scope()


def generate():
    parser = argparse.ArgumentParser(description='CATS code generator')
    parser.add_argument('-i', '--input', help='the input CATS file', required=True)
    args = parser.parse_args()

    file_parser = MultiFileParser()
    file_parser.parse(args.input)

    printer = pprint.PrettyPrinter(width=140)
    printer.pprint('*** *** ***')
    type_descriptors = file_parser.cats_parser.type_descriptors()
    for key in type_descriptors:
        printer.pprint((key, type_descriptors[key]))


generate()
