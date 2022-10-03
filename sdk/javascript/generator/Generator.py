#!/usr/bin/python

from pathlib import Path

from catparser.DisplayType import DisplayType
from catparser.generators.util import build_factory_map, extend_models

from .EnumTypeFormatter import EnumTypeFormatter
from .FactoryFormatter import FactoryClassFormatter, FactoryFormatter
from .PodTypeFormatter import PodTypeFormatter
from .printers import BuiltinPrinter, create_pod_printer
from .StructTypeFormatter import StructFormatter
from .TypeFormatter import TypeFormatter


def create_printer(descriptor, name, is_pod):
	return (create_pod_printer if is_pod else BuiltinPrinter)(descriptor, name)


def to_type_formatter_instance(ast_model):
	type_formatter_class = {
		DisplayType.STRUCT: StructFormatter,
		DisplayType.ENUM: EnumTypeFormatter,
		DisplayType.BYTE_ARRAY: PodTypeFormatter,
		DisplayType.INTEGER: PodTypeFormatter
	}[ast_model.display_type]

	return type_formatter_class(ast_model)


def generate_files(ast_models, output_directory: Path):
	factory_map = build_factory_map(ast_models)
	extend_models(ast_models, create_printer)

	output_directory.mkdir(exist_ok=True)

	requires_transform = False
	for ast_model in ast_models:
		if DisplayType.STRUCT == ast_model.display_type and ast_model.comparer:
			if any('ripemd_keccak_256' == transform for (_, transform) in ast_model.comparer):
				requires_transform = True

	with open(output_directory / 'models.js', 'w', encoding='utf8', newline='\n') as output_file:
		output_file.write(
			'''/* eslint-disable max-len, object-property-newline, no-underscore-dangle, no-use-before-define */

import BaseValue from '../BaseValue.js';
import ByteArray from '../ByteArray.js';
import BufferView from '../utils/BufferView.js';
import Writer from '../utils/Writer.js';
import * as arrayHelpers from '../utils/arrayHelpers.js';
import * as converter from '../utils/converter.js';
'''
		)

		if requires_transform:
			output_file.write('import { ripemdKeccak256 } from \'../utils/transforms.js\';\n')

		output_file.write('\n')

		for ast_model in ast_models:
			generator = TypeFormatter(to_type_formatter_instance(ast_model))
			output_file.write(str(generator))
			output_file.write('\n')

		factories = []
		factory_names = []
		for ast_model in ast_models:
			if DisplayType.STRUCT == ast_model.display_type and ast_model.is_abstract:
				factory_formatter = FactoryFormatter(factory_map, ast_model)
				factory_generator = FactoryClassFormatter(factory_formatter)
				factory_names.append(factory_formatter.typename)
				factories.append(str(factory_generator))

		output_file.write('\n'.join(factories))


class Generator:
	@staticmethod
	def generate(ast_models, output):
		print(f'python catbuffer generator called with output: {output}')
		generate_files(ast_models, Path(output))
