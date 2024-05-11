from collections import namedtuple
from itertools import filterfalse

from catparser.DisplayType import DisplayType

from generator.format import indent
from generator.name_formatting import underline_name
from generator.StructTypeFormatter import filter_size_if_first, is_bound_size, is_computed, is_const, is_reserved

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor

DescriptorFields = namedtuple('DescriptorFields', ['required', 'optional'])


class StructFormatter(AbstractTypeFormatter):
	# pylint: disable=too-many-public-methods

	def __init__(self, ast_model, factory_ast_model, name_to_display_type_map):
		super().__init__()

		self.struct = ast_model
		self.base_struct = factory_ast_model
		self.name_to_display_type_map = name_to_display_type_map

	def non_reserved_fields(self):
		non_const_fields = filterfalse(is_const, self.struct.fields)
		return filter_size_if_first(
			filterfalse(is_computed, filterfalse(is_bound_size, filterfalse(is_reserved, non_const_fields)))
		)

	def _is_inherited_field(self, field):
		if not self.base_struct:
			return False

		return bool(
			next((base_struct_field for base_struct_field in self.base_struct.fields if field.name == base_struct_field.name), None)
		)

	def descriptor_fields(self):
		required_fields = []
		optional_fields = []

		for field in self.non_reserved_fields():
			if self._is_inherited_field(field):
				continue

			if field.display_type.is_array or field.is_conditional:
				optional_fields.append(field)
			else:
				required_fields.append(field)

		return DescriptorFields(required_fields, optional_fields)

	def _get_documentation_type_simple(self, field):
		# where possible, use an sdk type instead of a models generated type
		sdk_type_override_map = {
			'Address': 'Address',
			'UnresolvedAddress': 'Address',
			'Hash256': 'Hash256',
			'PublicKey': 'PublicKey',
			'VotingPublicKey': 'PublicKey'
		}

		# `field` might be a field or a string
		typename = field.field_type if hasattr(field, 'field_type') else field

		if typename in sdk_type_override_map:
			return sdk_type_override_map[typename]

		if DisplayType.STRUCT == self.name_to_display_type_map.get(typename, DisplayType.UNSET):
			return f'{typename}Descriptor'

		return None

	def _get_documentation_type(self, field):
		documentation_type = self._get_documentation_type_simple(field)
		if documentation_type:
			return documentation_type

		if DisplayType.INTEGER == field.display_type:
			return 'bigint' if field.field_type.size > 4 else 'number'

		if field.display_type.is_array:
			# check if the array represents a string, i.e. uint8[]
			if hasattr(field.field_type.element_type, 'display_type'):
				if DisplayType.INTEGER == field.field_type.element_type.display_type and 1 == field.field_type.element_type.size:
					return 'Uint8Array|string|undefined'

			element_type = self._get_documentation_type_simple(field.field_type.element_type)
			if not element_type:
				element_type = f'models.{field.field_type.element_type}'

			return f'{element_type}[]|undefined'

		return f'models.{field.field_type}'

	def _get_unwrap_expression(self, field):
		field_documentation_type = self._get_documentation_type(field)

		if field_documentation_type.endswith('Descriptor'):
			return '.toMap()'

		if field_documentation_type.endswith('Descriptor[]|undefined'):
			return '.map(descriptor => descriptor.toMap())'

		return ''

	@property
	def typename(self):
		return f'{self.struct.name}Descriptor'

	def get_class_documentation(self):
		return f'Type safe descriptor used to generate a descriptor map for {self.typename}.\n\n{str(self.struct.comment)}'

	@staticmethod
	def argument_name(field):
		return field.extensions.printer.name

	def get_ctor_descriptor(self):
		descriptor_fields = self.descriptor_fields()

		body = 'this.rawDescriptor = {\n'

		initializer_list = []
		if self.struct.factory_type:
			initializer_list.append(f'type: \'{underline_name(self.struct.name)}\'')

		for field in descriptor_fields.required:
			argument_name = self.argument_name(field)
			unwrap_expression = self._get_unwrap_expression(field)
			if unwrap_expression:
				initializer_list.append(f'{argument_name}: {argument_name}{unwrap_expression}')
			else:
				initializer_list.append(argument_name)

		body += indent(',\n'.join(initializer_list))
		body += '};\n'

		for field in descriptor_fields.optional:
			argument_name = self.argument_name(field)
			unwrap_expression = self._get_unwrap_expression(field)

			body += '\n'
			body += f'if ({argument_name})\n'
			body += indent(f'this.rawDescriptor.{argument_name} = {argument_name}{unwrap_expression};')

		descriptor = MethodDescriptor(
			body=body,
			arguments=[
				self.argument_name(field) for field in descriptor_fields.required
			] + [f'{self.argument_name(field)} = undefined' for field in descriptor_fields.optional])
		descriptor.documentation.append(f'Creates a descriptor for {self.struct.name}.')

		for field in descriptor_fields.required:
			field_documentation_type = self._get_documentation_type(field)
			descriptor.documentation.append(f'@param {{{field_documentation_type}}} {self.argument_name(field)} {field.comment}')

		for field in descriptor_fields.optional:
			field_documentation_type = self._get_documentation_type(field)
			if not field_documentation_type.endswith('|undefined'):
				field_documentation_type += '|undefined'

			descriptor.documentation.append(f'@param {{{field_documentation_type}}} {self.argument_name(field)} {field.comment}')

		return descriptor

	def get_to_map_descriptor(self):
		descriptor = MethodDescriptor(method_name='toMap', body='return this.rawDescriptor;')
		descriptor.documentation += [
			'Builds a representation of this descriptor that can be passed to a factory function.',
			'@returns {object} Descriptor that can be passed to a factory function.'
		]
		return descriptor
