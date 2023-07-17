from enum import Enum
from itertools import filterfalse

from catparser.DisplayType import DisplayType

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import indent
from .name_formatting import fix_size_name, lang_field_name


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
	if not condition:
		return statements

	if '\n' not in statements[:-1]:
		return f'{condition}\n{indent(statements)}\n'

	return f'{condition} {{\n{indent(statements)}}}\n'


def filter_size_if_first(fields_iter):
	first_field = next(fields_iter, None)
	if 'size' == first_field.name:
		pass
	else:
		yield first_field

	for field in fields_iter:
		yield field


class DeserializerMode(Enum):
	USE_DEFAULT = 1
	USE_ALIGNED = 2
	USE_UNALIGNED = 3


class StructFormatter(AbstractTypeFormatter):
	# pylint: disable=too-many-public-methods

	def __init__(self, ast_model):
		super().__init__()

		self.struct = ast_model

	def non_const_fields(self):
		return filterfalse(is_const, self.struct.fields)

	def const_fields(self):
		return filter(is_const, self.struct.fields)

	def non_reserved_fields(self):
		return filter_size_if_first(filterfalse(is_computed, filterfalse(is_bound_size, filterfalse(is_reserved, self.non_const_fields()))))

	def reserved_fields(self):
		return filter(is_reserved, self.non_const_fields())

	def computed_fields(self):
		return filter(is_computed, self.non_const_fields())

	@property
	def typename(self):
		return self.struct.name

	@staticmethod
	def field_name(field, object_name='this'):
		if is_computed(field):
			# add _computed postfix for easier filtering in bespoke code
			return f'{object_name}.{field.extensions.printer.name}Computed'

		return f'{object_name}._{field.extensions.printer.name}'

	@staticmethod
	def generate_class_field(field):
		default_value = field.extensions.printer.assign(field.value)
		return f'static {field.name} = {default_value};\n'

	@staticmethod
	def buffer_view(buffer_name, shift=0, limit=0):
		shift_str = f' + {shift}' if shift else ''
		limit_str = f', {limit}' if limit else ''
		return f'new Uint8Array({buffer_name}.buffer, {buffer_name}.byteOffset{shift_str}{limit_str})'

	def generate_type_hints(self):
		body = 'static TYPE_HINTS = {\n'
		hints = []
		for field in self.non_reserved_fields():
			if not field.extensions.printer.type_hint:
				continue

			hints.append(f'{field.extensions.printer.name}: \'{field.extensions.printer.type_hint}\'')

		body += indent(',\n'.join(hints))
		body += '};\n'
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
		for field in self.non_reserved_fields():
			const_field = self.get_paired_const_field(field)
			field_name = self.field_name(field)
			if const_field:
				body += f'{field_name} = {self.typename}.{const_field.name};\n'
			else:
				value = field.extensions.printer.get_default_value()
				if field.is_conditional:
					conditional = field.value
					condition_field_name = conditional.linked_field_name
					condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
					condition_model = condition_field.extensions.type_model

					# only initialize default implicit union field in constructor
					if f'{condition_model.name}.{conditional.value}' != condition_field.extensions.printer.get_default_value():
						value = 'null'  # needs to be null or else field will not be destination when copying descriptor properties

				body += f'{field_name} = {value};\n'

		body += '\n'.join(
			map(
				lambda field: f'{self.field_name(field)} = {field.value}; // reserved field',
				self.reserved_fields()
			)
		)

		if not body:
			return None

		return MethodDescriptor(body=body, arguments=arguments)

	def get_comparer_descriptor(self):
		if not self.struct.comparer:
			return None

		body = 'return [\n'
		for (property_name, transform) in self.struct.comparer:
			body += '\t'
			if not transform:
				body += f'this.{lang_field_name(property_name)}'
			else:
				body += f'{lang_field_name(transform).replace("_", "")}(this.{lang_field_name(property_name)}.bytes)'

			body += ',\n'

		body = body[:-2]  # strip trailing comma
		body += '\n];'

		return MethodDescriptor(body=body)

	def generate_condition(self, field, prefix_field=False):
		if not field.is_conditional:
			return ''

		conditional = field.value
		condition_field_name = conditional.linked_field_name

		# find condition field type
		condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
		condition_to_operator_map = {
			'equals': '===',
			'not equals': '!==',
			'not in': '!',
			'in': '',
		}
		condition_operator = condition_to_operator_map[conditional.operation]

		value = f'{conditional.value}'
		condition_model = condition_field.extensions.type_model
		yoda_value = value if DisplayType.INTEGER == condition_model.display_type else f'{condition_model.name}.{value}'
		field_prefix = 'this.' if prefix_field else ''

		# HACK: instead of handling dumb magic value in namespace parent_name, generate slightly simpler condition
		if prefix_field and DisplayType.UNSET != field.display_type:
			return f'if ({field_prefix}{lang_field_name(field.extensions.printer.name)})'

		display_condition_field_name = lang_field_name(condition_field_name)
		if conditional.operation in ['not in', 'in']:
			return f'if ({condition_operator}{field_prefix}{display_condition_field_name}.has({yoda_value}))'

		field_postfix = 'Computed' if prefix_field and is_computed(condition_field) else ''

		return f'if ({yoda_value} {condition_operator} {field_prefix}{display_condition_field_name}{field_postfix})'

	def get_sort_descriptor(self):
		body = ''
		is_last_sort_field_conditional = False
		for field in self.non_const_fields():
			field_value = self.field_name(field)

			sort = field.extensions.printer.sort(field_value)
			if not sort:
				continue

			condition = self.generate_condition(field, True)

			if is_computed(field):
				sort += 'Computed'

			body += indent_if_conditional(condition, f'{sort}\n')
			is_last_sort_field_conditional = bool(condition)

		# indent_if_conditional always adds a newline when there is a condition
		# if the last sortable field has a condition, the newline needs to be stripped to avoid a blank line before closing brace
		if is_last_sort_field_conditional:
			body = body[:-1]

		return MethodDescriptor(body=body)

	def initialize_with_null(self, field):
		# HACK: instead of handling dumb magic value in namespace parent_name, generate slightly simpler condition
		if DisplayType.UNSET != field.display_type:
			return True

		# any fields with computed conditions should be null initialized or else the size computation will fail
		condition_field = next(f for f in self.non_const_fields() if field.value.linked_field_name == f.name)
		return is_computed(condition_field)

	def generate_deserialize_field(self, field, deserializer_mode, arg_buffer_name=None):
		# pylint: disable=too-many-locals

		condition = self.generate_condition(field)

		buffer_name = arg_buffer_name or 'view'
		field_name = fix_size_name(field.extensions.printer.name)

		# half-hack: limit buffer to amount specified in size field
		buffer_load_name = buffer_name
		size_fields = field.extensions.size_fields
		if size_fields:
			assert len(size_fields) == 1, f'unexpected number of size_fields associated with {field.name}'
			buffer_load_name = f'view.window({lang_field_name(size_fields[0].name)})'

		use_custom_buffer_name = arg_buffer_name or size_fields
		if not use_custom_buffer_name:
			buffer_load_name = 'view.buffer'

		use_aligned_deserializer = self.struct.is_aligned
		if DeserializerMode.USE_DEFAULT != deserializer_mode:
			use_aligned_deserializer = DeserializerMode.USE_ALIGNED == deserializer_mode

		load = field.extensions.printer.load(buffer_load_name, use_aligned_deserializer)
		const_field = 'const ' if not condition else ''
		deserialize = f'{const_field}{field_name} = {load};\n'

		# hack: if there's passed buffer name it means it's a name of temporary buffer (used for coditionals)
		# don't generate adjust in such case
		adjust = '' if arg_buffer_name else f'{buffer_name}.shiftRight({field.extensions.printer.advancement_size()});\n'

		additional_statements = ''
		if is_reserved(field):
			additional_statements = f'if ({field.value} !== {field.extensions.printer.name})\n'
			additional_statements += indent(f'throw RangeError(`Invalid value of reserved field (${{{field.extensions.printer.name}}})`);')

		if self.struct.size == field.extensions.printer.name:
			additional_statements += f'view.shrink({field_name} - {field.extensions.printer.advancement_size()});\n'

		if is_bound_size(field) and field.is_size_reference:
			additional_statements += '// marking sizeof field\n'

		deserialize_field = deserialize + adjust + additional_statements

		if condition:
			value = field.extensions.printer.get_default_value()
			if self.initialize_with_null(field):
				value = 'null'

			condition = f'let {field.extensions.printer.name} = {value};\n' + condition

		return indent_if_conditional(condition, deserialize_field)

	def get_deserialize_descriptor_impl(self, deserializer_mode):  # pylint: disable=too-many-locals
		body = 'const view = new BufferView(payload);\n'

		# special treatment for condition-guarded fields,
		# where condition is behind the fields...
		processed_fields = set()
		queued_fields = {}
		for field in self.non_const_fields():
			if field.is_conditional:
				condition_field_name = field.value.linked_field_name

				if condition_field_name not in processed_fields:
					if condition_field_name not in queued_fields:
						queued_fields[condition_field_name] = []

						# assume same size and generate single dummy access
						comment = '// deserialize to temporary buffer for further processing'
						deserialize = f'const {field.extensions.printer.name}Temporary = {field.extensions.printer.load()};'
						temporary_buffer = create_temporary_buffer_name(condition_field_name)
						temporary_buffer_limit = f'{field.extensions.printer.name}Temporary.size'
						temporary = f'const {temporary_buffer} = view.window({temporary_buffer_limit});'
						adjust = f'view.shiftRight({temporary_buffer_limit}); // skip temporary'
						body += comment + '\n' + deserialize + '\n' + temporary + '\n' + adjust + '\n\n'

					# queue field for re-reading it from temporary buffer
					queued_fields[condition_field_name].append({'field': field})
					continue

			deserialized_field = self.generate_deserialize_field(field, deserializer_mode)
			body += deserialized_field
			processed_fields.add(field.name)

			# if conditional field has been processed, process queued fields
			for conditioned in queued_fields.get(field.name, []):
				body += self.generate_deserialize_field(
					conditioned['field'],
					deserializer_mode,
					create_temporary_buffer_name(field.name),
				)

		# create call to ctor
		body += '\n'
		body += f'const instance = new {self.typename}();\n'

		for field in self.non_reserved_fields():
			field_name = self.field_name(field, 'instance')
			body += f'{field_name} = {field.extensions.printer.name};\n'

		body += 'return instance;'
		return MethodDescriptor(body=body)

	def get_deserialize_descriptor(self):
		if not self.struct.requires_unaligned:
			return self.get_deserialize_descriptor_impl(DeserializerMode.USE_DEFAULT)

		return self.get_deserialize_descriptor_impl(DeserializerMode.USE_UNALIGNED)

	def get_deserialize_aligned_descriptor(self):
		if not self.struct.requires_unaligned:
			return None

		return self.get_deserialize_descriptor_impl(DeserializerMode.USE_ALIGNED)

	def generate_serialize_field(self, field):
		condition = self.generate_condition(field, True)

		field_value = ''
		field_comment = ''

		# bound fields are the size / count / sizeof fields that are bound to either object or array
		bound_field = field.extensions.bound_field
		if is_bound_size(field):
			bound_field_name = self.field_name(bound_field)
			field_comment = f' // bound: {field.name}'

			if bound_field.display_type.is_array:
				if field.name.endswith('_count') or not bound_field.field_type.is_byte_constrained:
					field_value = f'{bound_field_name}.length'

					bound_condition = self.generate_condition(bound_field, True)
					if condition and bound_condition:
						raise RuntimeError('do not know yet how to generate both conditions')

					# HACK: create inline if condition (for NEM namespace purposes)
					if bound_condition:
						condition_value = bound_field.value.value
						field_value = f'({bound_field_name} ? {field_value} : {condition_value})'
				else:
					field_value = bound_field.extensions.printer.get_size()
			elif field.is_size_reference:
				field_value = bound_field.extensions.printer.get_size()
		else:
			field_value = self.field_name(field)

		serialize_line = None
		if DisplayType.TYPED_ARRAY == field.display_type:
			serialize_field = field.extensions.printer.store(field_value, 'buffer')
			serialize_line = f'{serialize_field};{field_comment}\n'
		else:
			serialize_field = field.extensions.printer.store(field_value)
			serialize_line = f'buffer.write({serialize_field});{field_comment}\n'

		return indent_if_conditional(condition, serialize_line)

	def get_serialize_descriptor(self):
		body = 'const buffer = new Writer(this.size);\n'

		# if first field is size replace serializer with custom one (to access builder .size() instead)
		fields_iter = self.non_const_fields()
		first_field = next(fields_iter)
		if self.struct.size == first_field.extensions.printer.name:
			body += f'buffer.write(converter.intToBytes(this.size, {first_field.size}, false));\n'
		else:
			body += self.generate_serialize_field(first_field)

		for field in fields_iter:
			body += self.generate_serialize_field(field)

		body += 'return buffer.storage;'
		return MethodDescriptor(body=body)

	def generate_size_field(self, field):
		condition = self.generate_condition(field, True)
		size_field = field.extensions.printer.get_size()

		return indent_if_conditional(condition, f'size += {size_field};\n')

	def get_size_descriptor(self):
		body = 'let size = 0;\n'
		body += ''.join(map(self.generate_size_field, self.non_const_fields()))
		body += 'return size;'
		return MethodDescriptor(body=body)

	def create_getter_descriptor(self, field):
		method_name = f'get {field.extensions.printer.name}'
		body = f'return {self.field_name(field)};'

		if is_computed(field):
			method_name += 'Computed'

			sizeref = field.field_type.sizeref
			body = f'return this.{sizeref.property_name} ? this.{sizeref.property_name}.size + {sizeref.delta} : 0;'

		method_descriptor = MethodDescriptor(method_name=method_name, body=body)
		return method_descriptor

	def create_setter_descriptor(self, field):
		method_descriptor = MethodDescriptor(
			method_name=f'set {field.extensions.printer.name}',
			arguments=['value'],
			body=f'{self.field_name(field)} = value;',
		)
		return method_descriptor

	def get_getter_setter_descriptors(self):
		descriptors = []
		for field in self.non_reserved_fields():
			descriptors.append(self.create_getter_descriptor(field))
			descriptors.append(self.create_setter_descriptor(field))

		for field in self.computed_fields():
			descriptors.append(self.create_getter_descriptor(field))

		return descriptors

	def generate_str_field(self, field):
		condition = self.generate_condition(field, True)
		field_to_string = field.extensions.printer.to_string(self.field_name(field))
		value = None
		if DisplayType.TYPED_ARRAY == field.display_type:
			value = f'[${{{field_to_string}}}]'
		elif DisplayType.BYTE_ARRAY == field.display_type:
			value = f'hex(${{{field_to_string}}})'
		else:
			value = f'${{{field_to_string}}}'
		return indent_if_conditional(condition, f'result += `{field.extensions.printer.name}: {value}, `;\n')

	def get_str_descriptor(self):
		body = 'let result = \'(\';\n'
		body += ''.join(map(self.generate_str_field, self.non_reserved_fields()))
		body += 'result += \')\';\n'
		body += 'return result;'
		return MethodDescriptor(body=body)
