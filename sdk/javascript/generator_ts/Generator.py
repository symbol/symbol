#!/usr/bin/python

from pathlib import Path

from catparser.DisplayType import DisplayType
from catparser.generators.util import extend_models

from generator.name_formatting import lang_field_name

from .StructTypeFormatter import StructFormatter
from .TypeFormatter import TypeFormatter


class Printer:
	def __init__(self, name):
		# printer.name is 'fixed' field name
		self.name = lang_field_name(name)


def create_printer(_descriptor, name, _is_pod):
	return Printer(name)


def find_factory_ast_model(ast_model, ast_models):
	if not ast_model.factory_type:
		return None

	return next(factory_ast_model for factory_ast_model in ast_models if ast_model.factory_type == factory_ast_model.name)


def get_adjusted_display_type(ast_model):
	# by default, all structs are assumed to have descriptors
	# change the display types of the following structs to bypass descriptor mapping
	model_only_structs = [
		'NonVerifiableTransaction',
		'EmbeddedTransaction',
		'Cosignature'
	]

	return DisplayType.UNSET if ast_model.name in model_only_structs else ast_model.display_type


def to_type_formatter_instance(ast_model, ast_models, name_to_display_type_map):
	if DisplayType.STRUCT == ast_model.display_type and not ast_model.is_abstract:
		factory_ast_model = find_factory_ast_model(ast_model, ast_models)
		if not factory_ast_model or 'Transaction' == factory_ast_model.name:
			return StructFormatter(ast_model, factory_ast_model, name_to_display_type_map)

	return None


def generate_files(ast_models, output_directory: Path):
	extend_models(ast_models, create_printer)

	output_directory.mkdir(exist_ok=True)

	with open(output_directory / 'models_ts.js', 'w', encoding='utf8', newline='\n') as output_file:
		output_file.write(
			'''/* eslint-disable max-len */

import { Address } from './Network.js';
import * as models from './models.js';
import { Hash256, PublicKey } from '../CryptoTypes.js';'''
		)

		output_file.write('\n')

		name_to_display_type_map = {ast_model.name: get_adjusted_display_type(ast_model) for ast_model in ast_models}

		for ast_model in ast_models:
			type_formatter = to_type_formatter_instance(ast_model, ast_models, name_to_display_type_map)
			if not type_formatter:
				continue

			generator = TypeFormatter(type_formatter)
			output_file.write('\n')
			output_file.write(str(generator))


class Generator:
	@staticmethod
	def generate(ast_models, output):
		print(f'python catbuffer generator called with output: {output}')
		generate_files(ast_models, Path(output))
