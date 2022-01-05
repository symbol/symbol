import argparse
import importlib
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


def _validate(raw_type_descriptors, stage, mode):
	validator = AstValidator(raw_type_descriptors)
	validator.set_validation_mode(mode)
	validator.validate()
	if validator.errors:
		print_error(f'[ERRORS DETECTED AT STAGE {stage}]')
		for error in validator.errors:
			print(f' + {error}')

		sys.exit(2)


def _load_generator_class(generator_full_name):
	generator_class_name = Path(generator_full_name).suffix[1:]

	print()
	print(f'loading generator {generator_class_name} from {generator_full_name}')

	generator_module = importlib.import_module(generator_full_name)
	generator_class = getattr(generator_module, generator_class_name)

	print(f'loaded generator: {generator_class}')
	print()

	return generator_class


class NoAliasDumper(yaml.SafeDumper):
	def ignore_aliases(self, data):
		return True


def main():
	parser = argparse.ArgumentParser(
		prog=None if globals().get('__spec__') is None else f'python -m {__spec__.name.partition(".")[0]}',
		description='CATS code generator'
	)
	parser.add_argument('-s', '--schema', help='input CATS file', required=True)
	parser.add_argument('-i', '--include', help='schema root directory', required=True)
	parser.add_argument('-o', '--output', help='yaml output file')
	parser.add_argument('-g', '--generator', help='generator class to use to produce output files (defaults to YAML output)')
	parser.add_argument('-q', '--quiet', help='do not print type descriptors to console', action='store_true')
	args = parser.parse_args()

	file_parser = LarkMultiFileParser()
	file_parser.set_include_path(args.include)

	try:
		raw_type_descriptors = file_parser.parse(args.schema)
	except (AstException, OSError) as ex:
		print_error(str(ex))
		sys.exit(1)

	processor = AstPostProcessor(raw_type_descriptors)

	_validate(raw_type_descriptors, 'PRE EXPANSION', AstValidator.Mode.PRE_EXPANSION)

	processor.apply_attributes()
	processor.expand_named_inlines()
	processor.expand_unnamed_inlines()

	_validate(raw_type_descriptors, 'POST EXPANSION', AstValidator.Mode.POST_EXPANSION)

	type_descriptors = [model.to_legacy_descriptor() for model in processor.type_descriptors]

	if not args.quiet:
		# dump parsed type descriptors to console
		yaml.dump(type_descriptors, sys.stdout, Dumper=NoAliasDumper)

	if args.output:
		if args.generator:
			generator_class = _load_generator_class(args.generator)
			generator_class.generate(processor.type_descriptors, args.output)
		else:
			# save YAML to file
			with open(args.output, 'wt', encoding='utf8') as out:
				yaml.dump(type_descriptors, out, Dumper=NoAliasDumper)


if '__main__' == __name__:
	main()
