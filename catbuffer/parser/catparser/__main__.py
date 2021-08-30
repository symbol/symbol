import argparse
import os
import pprint

from .CatsParser import CatsParser

try:
    from generators.All import AVAILABLE_GENERATORS
except ImportError:
    AVAILABLE_GENERATORS = {}


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


def _generate_output(generator_name, output_path, schema, options):
    generator_class = AVAILABLE_GENERATORS[generator_name]
    os.makedirs(output_path, exist_ok=True)
    generator = generator_class(schema, options)
    for generated_descriptor in generator:
        output_filename = os.path.join(output_path, generated_descriptor.filename)
        with open(output_filename, 'w', newline='\n') as output_file:
            for line in generated_descriptor.code:
                output_file.write('%s\n' % line)


def main():
    parser = argparse.ArgumentParser(
        prog=None if globals().get('__spec__') is None else 'python -m {}'.format(__spec__.name.partition('.')[0]),
        description='CATS code generator'
    )
    parser.add_argument('-s', '--schema', help='input CATS file', required=True)
    parser.add_argument('-o', '--output', help='output directory, if not provided, _generated/{generator} is used')
    parser.add_argument('-i', '--include', help='schema root directory', default='./schemas')

    generators_list = list(AVAILABLE_GENERATORS.keys())
    parser.add_argument('-g', '--generator', help='generator to use to produce output files', choices=generators_list)
    parser.add_argument('-c', '--copyright', help='file containing copyright data to use with output files', default='../HEADER.inc')
    args = parser.parse_args()

    file_parser = MultiFileParser()
    file_parser.set_include_path(args.include)
    file_parser.parse(args.schema)

    # console output the parsed schema
    printer = pprint.PrettyPrinter(width=140)
    printer.pprint('*** *** ***')
    type_descriptors = file_parser.cats_parser.type_descriptors()
    for key in type_descriptors:
        printer.pprint((key, type_descriptors[key]))

    # generate and output code
    if args.generator:
        output_path = args.output
        if output_path is None:
            output_path = os.path.join('_generated', args.generator)

        _generate_output(args.generator, output_path, type_descriptors, {'copyright': args.copyright})


if '__main__' == __name__:
    main()
