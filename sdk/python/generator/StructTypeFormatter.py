from itertools import filterfalse

from catparser.DisplayType import DisplayType

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import indent
from .name_formatting import fix_size_name


def is_reserved(field):
	return 'reserved' == field.disposition


def is_bound_size(field):
	return field.extensions.bound_field is not None


def is_const(field):
	return field.is_const


def is_computed(field):
	return hasattr(field.field_type, 'sizeref') and field.field_type.sizeref


def create_temporary_buffer_name(name):
	return f'{name}_condition'


def indent_if_conditional(condition, statements):
	return statements if not condition else condition + indent(statements)


def filter_size_if_first(fields_iter):
	first_field = next(fields_iter, None)
	if 'size' == first_field.name:
		pass
	else:
		yield first_field

	yield from fields_iter


class StructFormatter(AbstractTypeFormatter):
	# pylint: disable=too-many-public-methods

	def __init__(self, ast_model, factory_ast_model=None):
		super().__init__()

		self.struct = ast_model
		self.base_struct = factory_ast_model

	def non_const_fields(self, include_inherited=True):
		fields = filterfalse(is_const, self.struct.fields)
		return self._filter_inherited_fields(fields, include_inherited)

	def const_fields(self):
		return filter(is_const, self.struct.fields)

	def non_reserved_fields(self, include_inherited=True):
		fields = filter_size_if_first(
			filterfalse(is_computed, filterfalse(is_bound_size, filterfalse(is_reserved, self.non_const_fields())))
		)
		return self._filter_inherited_fields(fields, include_inherited)

	def reserved_fields(self, include_inherited=True):
		fields = filter(is_reserved, self.non_const_fields())
		return self._filter_inherited_fields(fields, include_inherited)

	def computed_fields(self, include_inherited=True):
		fields = filter(is_computed, self.non_const_fields())
		return self._filter_inherited_fields(fields, include_inherited)

	def _is_inherited_field(self, field):
		if not self.base_struct:
			return False

		return bool(
			next((base_struct_field for base_struct_field in self.base_struct.fields if field.name == base_struct_field.name), None)
		)

	def _filter_inherited_fields(self, fields, include_inherited):
		if include_inherited:
			return fields

		return filterfalse(self._is_inherited_field, fields)

	@property
	def typename(self):
		return self.struct.name

	@property
	def is_type_abstract(self):
		return self.struct.is_abstract

	def get_base_class(self):
		return '' if not self.struct.factory_type else f'({self.struct.factory_type})'

	@staticmethod
	def field_name(field, object_name='self'):
		if is_computed(field):
			# add _computed postfix for easier filtering in bespoke code
			return f'{object_name}.{field.extensions.printer.name}_computed'

		return f'{object_name}._{field.extensions.printer.name}'

	@staticmethod
	def generate_class_field(field):
		default_value = field.extensions.printer.assign(field.value)
		return f'{field.name}: {field.extensions.printer.get_type()} = {default_value}'

	def generate_type_hints(self):
		body = 'TYPE_HINTS = {\n'
		hints = []

		if self.base_struct:
			hints.append(f'**{self.base_struct.name}.TYPE_HINTS')

		for field in self.non_reserved_fields(include_inherited=False):
			if not field.extensions.printer.type_hint:
				continue

			hints.append(f'\'{field.extensions.printer.name}\': \'{field.extensions.printer.type_hint}\'')

		body += indent(',\n'.join(hints))
		body += '}\n'
		return body

	def get_fields(self):
		return list(map(self.generate_class_field, self.const_fields())) + [self.generate_type_hints()]

	def get_paired_const_field(self, field):
		for const_field in self.const_fields():
			if const_field.name.lower().endswith(field.name):
				return const_field
		return None

	def get_ctor_descriptor(self):
		arguments = []

		body = ''
		if self.base_struct:
			body += 'super().__init__()\n'

		# include inherited fields because those paired with constants need to be set
		for field in self.non_reserved_fields():
			const_field = self.get_paired_const_field(field)
			field_name = self.field_name(field)
			if const_field:
				body += f'{field_name} = {self.typename}.{const_field.name}\n'
			elif not self._is_inherited_field(field):
				value = field.extensions.printer.get_default_value()
				if field.is_conditional:
					conditional = field.value
					condition_field_name = conditional.linked_field_name
					condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
					condition_model = condition_field.extensions.type_model

					# only initialize default implicit union field in constructor
					if f'{condition_model.name}.{conditional.value}' != condition_field.extensions.printer.get_default_value():
						value = 'None'

				body += f'{field_name} = {value}\n'

		body += '\n'.join(
			map(
				lambda field: f'{self.field_name(field)} = {field.value}  # reserved field',
				self.reserved_fields(include_inherited=False)
			)
		)

		if not body:
			return None

		return MethodDescriptor(body=body, arguments=arguments)

	def get_comparer_descriptor(self):
		if not self.struct.comparer:
			return None

		body = ''
		if any('ripemd_keccak_256' == transform for (_, transform) in self.struct.comparer):
			body += 'from ..Transforms import ripemd_keccak_256  # pylint: disable=import-outside-toplevel\n\n'

		body += 'return (\n'
		for (property_name, transform) in self.struct.comparer:
			body += '\t'
			if not transform:
				body += f'self.{property_name} if not isinstance(self.{property_name}, Enum) else self.{property_name}.value'
			else:
				body += f'{transform}(self.{property_name}.bytes)'

			body += ',\n'

		body += ')'

		return MethodDescriptor(body=body)

	def generate_condition(self, field, prefix_field=False):
		if not field.is_conditional:
			return ''

		conditional = field.value
		condition_field_name = conditional.linked_field_name

		# find condition field type
		condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
		condition_to_operator_map = {
			'equals': '==',
			'not equals': '!=',
			'not in': 'not in',
			'in': 'in',
		}
		condition_operator = condition_to_operator_map[conditional.operation]

		value = f'{conditional.value}'
		condition_model = condition_field.extensions.type_model
		yoda_value = value if DisplayType.INTEGER == condition_model.display_type else f'{condition_model.name}.{value}'
		field_prefix = 'self.' if prefix_field else ''

		# HACK: instead of handling dumb magic value in namespace parent_name, generate slightly simpler condition
		if prefix_field and DisplayType.UNSET != field.display_type:
			return f'if {field_prefix}{field.name}:\n'

		field_postfix = '_computed' if prefix_field and is_computed(condition_field) else ''

		return f'if {yoda_value} {condition_operator} {field_prefix}{condition_field_name}{field_postfix}:\n'

	def get_sort_descriptor(self):
		body = ''
		for field in self.non_const_fields():
			field_value = self.field_name(field)

			sort = field.extensions.printer.sort(field_value)
			if not sort:
				continue

			condition = self.generate_condition(field, True)

			if is_computed(field):
				sort += '_computed'

			body += indent_if_conditional(condition, f'{sort}\n')

		if not body:
			body = 'pass'

		return MethodDescriptor(body=body)

	def generate_deserialize_field(self, field, arg_buffer_name=None):
		condition = self.generate_condition(field)

		buffer_name = arg_buffer_name or 'buffer'
		field_name = fix_size_name(field.extensions.printer.name)

		# half-hack: limit buffer to amount specified in size field
		buffer_load_name = buffer_name
		size_fields = field.extensions.size_fields
		if size_fields:
			assert len(size_fields) == 1, f'unexpected number of size_fields associated with {field.name}'
			buffer_load_name = f'buffer[:{size_fields[0].name}]'

		use_custom_buffer_name = arg_buffer_name or size_fields
		load = field.extensions.printer.load(buffer_load_name) if use_custom_buffer_name else field.extensions.printer.load()
		deserialize = f'{field_name} = {load}'
		adjust = f'{buffer_name} = {buffer_name}[{field.extensions.printer.advancement_size()}:'

		if self.struct.size == field.extensions.printer.name:
			adjust += field_name

		adjust += ']'

		additional_statements = ''
		if is_reserved(field):
			assert_message = f'f\'Invalid value of reserved field ({{{field.extensions.printer.name}}})\''
			additional_statements = f'assert {field.extensions.printer.name} == {field.value}, {assert_message}\n'

		if is_bound_size(field) and field.is_size_reference:
			additional_statements += '# marking sizeof field\n'

		deserialize_field = deserialize + '\n' + adjust + '\n' + additional_statements

		if condition:
			condition = f'{field.extensions.printer.name} = None\n' + condition

		return indent_if_conditional(condition, deserialize_field)

	def get_deserialize_descriptor(self):
		body = ''
		if not self.is_type_abstract:
			body = 'buffer = memoryview(payload)\n'
			body += f'instance = {self.typename}()\n'
		elif not any('size' == field.name for field in self.non_const_fields(include_inherited=False)):
			# if struct does not have size field, assume it fills entire buffer
			body += 'size_ = len(buffer)\n'

		if self.base_struct:
			body += f'(window_start, window_end) = {self.base_struct.name}._deserialize(buffer, instance)\n'
			body += 'buffer = buffer[window_start:window_end]\n'

		# special treatment for condition-guarded fields,
		# where condition is behind the fields...
		processed_fields = set()
		queued_fields = {}
		for field in self.non_const_fields(include_inherited=False):
			if field.is_conditional:
				conditional = field.value
				condition_field_name = conditional.linked_field_name

				if condition_field_name not in processed_fields:
					if condition_field_name not in queued_fields:
						queued_fields[condition_field_name] = []

						# assume same size and generate single dummy access
						comment = '# deserialize to temporary buffer for further processing'
						deserialize = f'{field.extensions.printer.name}_temporary = {field.extensions.printer.load()}'
						temporary_buffer = create_temporary_buffer_name(condition_field_name)
						temporary = f'{temporary_buffer} = buffer[:{field.extensions.printer.name}_temporary.size]'
						adjust = f'buffer = buffer[{field.extensions.printer.name}_temporary.size:]'
						body += comment + '\n' + deserialize + '\n' + temporary + '\n' + adjust + '\n\n'

					# queue field for re-reading it from temporary buffer
					queued_fields[condition_field_name].append({'field': field})
					continue

			deserialized_field = self.generate_deserialize_field(field)
			body += deserialized_field
			processed_fields.add(field.name)

			# if conditional field has been processed, process queued fields
			for conditioned in queued_fields.get(field.name, []):
				body += self.generate_deserialize_field(
					conditioned['field'],
					create_temporary_buffer_name(field.name),
				)

		# set fields
		body += '\n'
		body += '# pylint: disable=protected-access\n'

		for field in self.non_reserved_fields(include_inherited=False):
			field_name = self.field_name(field, 'instance')
			body += f'{field_name} = {field.extensions.printer.name}\n'

		if not self.is_type_abstract:
			body += 'return instance'
		else:
			body += 'return (size_ - len(buffer), size_)'

		return MethodDescriptor(body=body)

	def generate_serialize_field(self, field):
		condition = self.generate_condition(field, True)

		field_value = ''
		field_comment = ''

		# bound fields are the size / count / sizeof fields that are bound to either object or array
		bound_field = field.extensions.bound_field
		if is_bound_size(field):
			bound_field_name = self.field_name(bound_field)
			field_comment = f'  # {field.name}'

			if bound_field.display_type.is_array:
				if field.name.endswith('_count') or not bound_field.field_type.is_byte_constrained:
					field_value = f'len({bound_field_name})'

					bound_condition = self.generate_condition(bound_field, True)
					if condition and bound_condition:
						raise RuntimeError('do not know yet how to generate both conditions')

					# HACK: create inline if condition (for NEM namespace purposes)
					if bound_condition:
						condition_value = bound_field.value.value
						field_value = f'({field_value} if {bound_field_name} is not None else {condition_value})'
				else:
					field_value = bound_field.extensions.printer.get_size()
			elif field.is_size_reference:
				field_value = bound_field.extensions.printer.get_size()
		else:
			field_value = self.field_name(field)

		serialize_field = field.extensions.printer.store(field_value) + field_comment

		return indent_if_conditional(condition, f'buffer += {serialize_field}\n')

	def generate_serialize_fields(self):
		body = ''

		# if first field is size replace serializer with custom one (to access builder .size() instead)
		fields_iter = self.non_const_fields(include_inherited=False)
		first_field = next(fields_iter)
		if self.struct.size == first_field.extensions.printer.name:
			body += f'buffer += self.size.to_bytes({first_field.size}, byteorder=\'little\', signed=False)\n'
		else:
			body += self.generate_serialize_field(first_field)

		for field in fields_iter:
			body += self.generate_serialize_field(field)

		return body

	def get_serialize_descriptor(self):
		body = 'buffer = bytearray()\n'

		if self.base_struct:
			body += 'super()._serialize(buffer)\n'

		if self.is_type_abstract:
			body += 'self._serialize(buffer)\n'
		else:
			body += self.generate_serialize_fields()

		body += 'return buffer'
		return MethodDescriptor(body=body)

	def get_serialize_protected_descriptor(self):
		if not self.is_type_abstract:
			return None

		body = self.generate_serialize_fields()
		return MethodDescriptor(body=body)

	def generate_size_field(self, field):
		condition = self.generate_condition(field, True)
		size_field = field.extensions.printer.get_size()

		return indent_if_conditional(condition, f'size += {size_field}\n')

	def get_size_descriptor(self):
		body = 'size = 0\n'
		if self.base_struct:
			body += 'size += super().size\n'

		body += ''.join(map(self.generate_size_field, self.non_const_fields(include_inherited=False)))
		body += 'return size'
		return MethodDescriptor(body=body)

	def create_getter_descriptor(self, field):
		method_name = field.extensions.printer.name
		body = f'return {self.field_name(field)}'
		if is_computed(field):
			method_name += '_computed'

			sizeref = field.field_type.sizeref
			body = f'return 0 if not self.{sizeref.property_name} else self.{sizeref.property_name}.size + {sizeref.delta}'

		method_descriptor = MethodDescriptor(method_name=method_name, body=body, result=field.extensions.printer.get_type())
		method_descriptor.annotations = ['@property']
		return method_descriptor

	def get_getter_descriptors(self):
		return list(map(self.create_getter_descriptor, self.non_reserved_fields(include_inherited=False))) + (
			list(map(self.create_getter_descriptor, self.computed_fields(include_inherited=False)))
		)

	def create_setter_descriptor(self, field):
		method_descriptor = MethodDescriptor(
			method_name=field.extensions.printer.name,
			arguments=[f'value: {field.extensions.printer.get_type()}'],
			body=f'{self.field_name(field)} = value',
		)
		method_descriptor.annotations = [f'@{field.extensions.printer.name}.setter']
		return method_descriptor

	def get_setter_descriptors(self):
		return list(map(self.create_setter_descriptor, self.non_reserved_fields(include_inherited=False)))

	def generate_str_field(self, field):
		condition = self.generate_condition(field, True)
		field_to_string = field.extensions.printer.to_string(self.field_name(field))
		field_to_string = field_to_string if '{' in field_to_string else f'{{{field_to_string}}}'
		return indent_if_conditional(condition, f'result += f\'{field.extensions.printer.name}: {field_to_string}, \'\n')

	def get_str_descriptor(self):
		body = 'result = \'(\'\n'

		if self.base_struct:
			body += 'result += super().__str__()\n'

		body += ''.join(map(self.generate_str_field, self.non_reserved_fields(include_inherited=False)))

		body += 'result += \')\'\n'
		body += 'return result'
		return MethodDescriptor(body=body)
