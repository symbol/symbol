# pylint: disable=too-few-public-methods
import argparse
import os
import pprint
from catparser.CatsParser import CatsParser
from generators.All import AVAILABLE_GENERATORS


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


def _generate_output(generator_name, directory, schema):
    generator_class = AVAILABLE_GENERATORS[generator_name]
    output_path = os.path.join(directory, generator_name)
    os.makedirs(output_path, exist_ok=True)
    generator = generator_class(schema)
    for generated_descriptor in generator:
        output_filename = os.path.join(output_path, generated_descriptor.filename)
        with open(output_filename, 'w', newline='\n') as output_file:
            for line in generated_descriptor.code:
                output_file.write('%s\n' % line)


def generate():
    parser = argparse.ArgumentParser(description='CATS code generator')
    parser.add_argument('-i', '--input', help='the input CATS file', required=True)
    generators_list = list(AVAILABLE_GENERATORS.keys())
    parser.add_argument('-d', '--dir', help='output directory', default='_generated')
    parser.add_argument('-g', '--generator', help='the generator to use to produce output files', choices=generators_list)
    args = parser.parse_args()

    file_parser = MultiFileParser()
    file_parser.parse(args.input)

    # console output the parsed schema
    printer = pprint.PrettyPrinter(width=140)
    printer.pprint('*** *** ***')
    type_descriptors = file_parser.cats_parser.type_descriptors()
    for key in type_descriptors:
        printer.pprint((key, type_descriptors[key]))

    # generate and output code
    if args.generator:
        _generate_output(args.generator, args.dir, type_descriptors)


generate()
