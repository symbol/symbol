"""Emits a Java class for each catbuffer ``struct`` AST model.
This is the heart of the generator. For each struct it produces a complete Java class with.
"""

from itertools import filterfalse

from catparser.DisplayType import DisplayType

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .field_filters import descriptor_candidate_fields, is_bound_size, is_computed, is_const, is_inherited_field, is_reserved
from .format import indent
from .name_formatting import capitalize, lang_field_name
from .printers import ArrayPrinter, BuiltinPrinter, IntPrinter, TypedArrayPrinter


def _temporary_buffer_name(name):
	# camelCase the schema field name so the temporary matches the surrounding Java locals (registrationTypeCondition,
	# not registration_typeCondition)
	return f'{lang_field_name(name)}Condition'


def _indent_if_conditional(condition, statements):
	if not condition:
		return statements

	return f'{condition} {{\n{indent(statements)}}}\n'


def _java_int_literal(value, is_long):
	try:
		ivalue = int(value)
	except (TypeError, ValueError):
		# Non-integer value (e.g. enum reference like 'Foo.BAR') — leave as-is.
		return str(value)

	if is_long:
		return f'{ivalue}L'

	# int-typed: fold a value above Integer.MAX_VALUE (e.g. a u32 0xFFFFFFFF "absent" sentinel) to its signed two's-complement form
	if ivalue > 0x7FFFFFFF:
		ivalue -= 1 << 32

	return str(ivalue)


class StructFormatter(AbstractTypeFormatter):
	# pylint: disable=too-many-public-methods

	def __init__(self, ast_model, factory_ast_model=None, factory_map=None, ast_models=None):
		super().__init__()
		self.struct = ast_model
		self.base_struct = factory_ast_model
		self.factory_map = factory_map or {}
		self._ast_models = ast_models or []

	# -- field iteration helpers --------------------------------------------------------------

	def non_const_fields(self, include_inherited=True):
		fields = filterfalse(is_const, self.struct.fields)
		return self._filter_inherited_fields(fields, include_inherited)

	def const_fields(self):
		return filter(is_const, self.struct.fields)

	def non_reserved_fields(self, include_inherited=True):
		fields = descriptor_candidate_fields(self.struct.fields)
		return self._filter_inherited_fields(fields, include_inherited)

	def reserved_fields(self, include_inherited=True):
		fields = filter(is_reserved, self.non_const_fields())
		return self._filter_inherited_fields(fields, include_inherited)

	def computed_fields(self, include_inherited=True):
		fields = filter(is_computed, self.non_const_fields())
		return self._filter_inherited_fields(fields, include_inherited)

	def _is_inherited_field(self, field):
		return is_inherited_field(field, self.base_struct)

	def _filter_inherited_fields(self, fields, include_inherited):
		if include_inherited:
			return fields
		return filterfalse(self._is_inherited_field, fields)

	# -- name accessors -----------------------------------------------------------------------

	@property
	def typename(self):
		return self.struct.name

	@property
	def is_type_abstract(self):
		return self.struct.is_abstract

	def get_implemented_interfaces(self):
		# Root structs extend CatbufferType (which itself implements Serializer); subclasses inherit the Serializer contract
		# through the base.
		if self.struct.comparer:
			return f'Comparable<{self.struct.name}>'

		return None

	def get_base_class(self):
		if self.struct.factory_type:
			return self.struct.factory_type

		return 'org.symbol.sdk.CatbufferType'

	def get_permits(self):
		"""For abstract structs, return the concrete subclass names so the emitted class can be sealed."""
		if not self.is_type_abstract:
			return []

		factory_descriptor = self.factory_map.get(self.struct.name)
		if factory_descriptor is None or not factory_descriptor.children:
			return []

		return [child.name for child in factory_descriptor.children]

	def field_name(self, field, object_name='this'):
		"""Java accessor for a struct field."""
		# pylint: disable=no-self-use
		printer_name = field.extensions.printer.name
		if is_computed(field):
			return f'{object_name}.get{capitalize(printer_name)}Computed()'

		return f'{object_name}.{printer_name}'

	def get_paired_const_field(self, field):
		# A const field whose name ends with the field name is the "auto-set" pair (e.g.
		# `transactionType` field paired with `TRANSACTION_TYPE` const on a concrete tx)
		return next((const_field for const_field in self.const_fields() if const_field.name.lower().endswith(field.name)), None)

	def get_fields(self):
		"""Emit ``public static final`` constants and instance fields."""
		lines = []
		for const_field in self.const_fields():
			value = const_field.extensions.printer.assign(const_field.value)
			java_type = const_field.extensions.printer.get_type()
			lines.append(f'public static final {java_type} {const_field.name} = {value};')

		# Instance fields. Abstract bases use ``protected`` so subclasses can read/write inherited fields via direct access.
		visibility = 'protected' if self.is_type_abstract else 'private'
		for field in self.non_reserved_fields(include_inherited=False):
			java_type = field.extensions.printer.get_type()
			lines.append(f'{visibility} {java_type} {field.extensions.printer.name};')

		for field in self.reserved_fields(include_inherited=False):
			java_type = field.extensions.printer.get_type()
			lines.append(f'{visibility} final {java_type} {field.extensions.printer.name} = {field.value}; // reserved field')

		# Static TYPE_HINTS map — drives the rule-based factory's per-field coercion; emitted as a flat (own + inherited) map.
		lines.append(self.generate_type_hints())

		return lines

	def generate_type_hints(self):
		"""Emit a ``public static final Map<String, String> TYPE_HINTS`` map literal."""
		hints = []
		for field in self.non_reserved_fields(include_inherited=True):
			type_hint = field.extensions.printer.type_hint
			if not type_hint:
				continue

			hints.append((field.extensions.printer.name, type_hint))

		decl = 'public static final java.util.Map<String, String> TYPE_HINTS'
		if not hints:
			return f'{decl} = java.util.Map.ofEntries();'

		entries = ',\n\t\t'.join(f'java.util.Map.entry("{name}", "{type_hint}")' for name, type_hint in hints)
		return f'{decl} = java.util.Map.ofEntries(\n\t\t{entries});'

	def get_ctor_descriptor(self):
		body_lines = []
		if self.base_struct:
			body_lines.append('super();')

		for field in self.non_reserved_fields():
			const_field = self.get_paired_const_field(field)
			field_ref = self.field_name(field)
			if const_field:
				body_lines.append(f'{field_ref} = {self.typename}.{const_field.name};')
			elif not self._is_inherited_field(field):
				value = field.extensions.printer.get_default_value()
				if field.is_conditional:
					conditional = field.value
					condition_field_name = conditional.linked_field_name
					condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
					condition_model = condition_field.extensions.type_model
					if f'{condition_model.name}.{conditional.value}' != condition_field.extensions.printer.get_default_value():
						value = 'null'

				body_lines.append(f'{field_ref} = {value};')

		if not body_lines:
			return None

		return MethodDescriptor(body='\n'.join(body_lines), arguments=[])

	def generate_condition(self, field, prefix_field=False):
		if not field.is_conditional:
			return ''

		conditional = field.value
		condition_field_name = conditional.linked_field_name
		condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.name)
		condition_to_operator = {
			'equals': '==',
			'not equals': '!=',
			'not in': '!',
			'in': '',
		}
		operator = condition_to_operator[conditional.operation]
		value = f'{conditional.value}'
		condition_model = condition_field.extensions.type_model
		if DisplayType.INTEGER == condition_model.display_type:
			yoda_value = _java_int_literal(value, condition_field.extensions.printer.is_long())
		else:
			yoda_value = f'{condition_model.name}.{value}'
		field_prefix = 'this.' if prefix_field else ''

		# HACK: nullable conditional fields use a null check
		if prefix_field and DisplayType.UNSET != field.display_type:
			return f'if (null != {field_prefix}{field.extensions.printer.name})'

		display_field = lang_field_name(condition_field_name)
		if conditional.operation in ['not in', 'in']:
			return f'if ({operator}{field_prefix}{display_field}.has({yoda_value}))'

		if prefix_field and is_computed(condition_field):
			# Computed fields are accessed via their generated getter (no underlying private field).
			display_field_ref = f'get{capitalize(display_field)}Computed()'
		else:
			display_field_ref = display_field

		# For enum equality use Java identity comparison
		if DisplayType.ENUM == condition_model.display_type:
			cmp = '==' if operator == '==' else '!='
			return f'if ({field_prefix}{display_field_ref} {cmp} {yoda_value})'

		return f'if ({yoda_value} {operator} {field_prefix}{display_field_ref})'

	def get_compare_to_descriptor(self):
		"""Emit a typed ``compareTo(Self other)`` method for structs declaring a comparer clause."""
		if not self.struct.comparer:
			return None

		steps = []
		for (property_name, transform) in self.struct.comparer:
			java_field = lang_field_name(property_name)
			if not transform:
				steps.append(f'this.{java_field}.compareTo(other.{java_field})')
			else:
				method = ''.join(p.capitalize() for p in transform.split('_'))
				method = method[0].lower() + method[1:]
				steps.append(
					f'java.util.Arrays.compareUnsigned(\n'
					f'\t\torg.symbol.sdk.utils.Transforms.{method}(this.{java_field}.bytes()),\n'
					f'\t\torg.symbol.sdk.utils.Transforms.{method}(other.{java_field}.bytes()))')

		declaration = 'final int c = ' if 2 == len(steps) else 'int c = '
		body_lines = []
		for index, step in enumerate(steps[:-1]):
			body_lines.append(f'{declaration if 0 == index else "c = "}{step};')
			body_lines.append('if (0 != c) return c;')

		body_lines.append(f'return {steps[-1]};')
		descriptor = MethodDescriptor(body='\n'.join(body_lines), arguments=[f'final {self.struct.name} other'])
		descriptor.result = 'int'
		descriptor.annotations = ['@Override']
		return descriptor

	def get_sort_descriptor(self):
		body_lines = []
		for field in self.non_const_fields():
			sort_stmt = field.extensions.printer.sort(self.field_name(field))
			if not sort_stmt:
				continue

			condition = self.generate_condition(field, True)
			if is_computed(field):
				sort_stmt += 'Computed'

			body_lines.append(_indent_if_conditional(condition, sort_stmt + '\n'))
		body = ''.join(body_lines)
		if not body:
			# nothing to sort — inherit CatbufferType.sort()'s no-op default instead of emitting a redundant override
			return None

		descriptor = MethodDescriptor(body=body)
		descriptor.annotations = ['@Override']  # overrides CatbufferType.sort()
		return descriptor

	# -- deserialize --------------------------------------------------------------------------

	def _initialize_with_null(self, field):
		if DisplayType.UNSET != field.display_type:
			return True

		condition_field = next(f for f in self.non_const_fields() if field.value.linked_field_name == f.name)
		return is_computed(condition_field)

	def generate_deserialize_field(self, field, arg_buffer_name=None):
		# pylint: disable=too-many-locals
		condition = self.generate_condition(field)

		field_name = field.extensions.printer.name

		size_fields = field.extensions.size_fields
		if size_fields:
			assert len(size_fields) == 1, f'unexpected number of size_fields associated with {field.name}'
			# bound the read to the declared size via a zero-copy window; the size local is a bound field, hence int
			# (see IntPrinter.is_long), so it needs no narrowing cast
			size_local = lang_field_name(size_fields[0].name)
			cursor = f'cursor.window({size_local})'
		elif arg_buffer_name:
			# reading from a temporary zero-copy BufferView window (condition-guarded fields)
			cursor = arg_buffer_name
		else:
			cursor = 'cursor'

		load = field.extensions.printer.load(cursor)
		java_type = field.extensions.printer.get_type()
		# When the field is conditional the local was already declared *above* the `if` block
		# (see the bottom of this method). Inside the if, only assign — don't redeclare.
		if condition:
			deserialize = f'{field_name} = {load};\n'
		else:
			deserialize = f'final {java_type} {field_name} = {load};\n'

		# advancement_size() is always an int: a fixed byte width, a .size()/length, or a bound size/count local (int per
		# IntPrinter.is_long). shiftRight takes an int, so no cast is needed.
		advancement = field.extensions.printer.advancement_size()
		adjust = '' if arg_buffer_name else f'cursor.shiftRight({advancement});\n'

		additional = ''
		if is_reserved(field):
			additional = (
				f'if ({field.value} != {field.extensions.printer.name})\n'
				f'\tthrow new IllegalArgumentException("Invalid value of reserved field (" + {field.extensions.printer.name} + ")");\n'
			)

		if self.struct.size == field.extensions.printer.name:
			additional += f'cursor.shrink((int) ({field_name} - ({field.extensions.printer.advancement_size()})));\n'

		deserialize_field = deserialize + adjust + additional

		if condition:
			value = field.extensions.printer.get_default_value()
			if self._initialize_with_null(field):
				value = 'null'
			condition = f'{java_type} {field.extensions.printer.name} = {value};\n' + condition

		return _indent_if_conditional(condition, deserialize_field)

	def _deserialize_body(self):
		body = ''
		if not self.is_type_abstract:
			# snapshot the caller's view into a private cursor: we read without advancing the passed view, so the
			# caller advances it by this object's size() (see ArrayHelpers / nested-struct loads)
			body = 'final org.symbol.sdk.utils.BufferView cursor = new org.symbol.sdk.utils.BufferView(buffer);\n'
			body += f'final {self.typename} instance = new {self.typename}();\n\n'

		if self.base_struct:
			body += f'{self.base_struct.name}.deserializeInto(cursor, instance);\n'

		# special treatment for condition-guarded fields where condition is behind the fields
		processed_fields = set()
		queued_fields = {}
		for field in self.non_const_fields(include_inherited=False):
			if field.is_conditional:
				condition_field_name = field.value.linked_field_name
				if condition_field_name not in processed_fields:
					if condition_field_name not in queued_fields:
						queued_fields[condition_field_name] = []
						java_type = field.extensions.printer.get_type()
						temp_name = f'{field.extensions.printer.name}Temporary'
						temp_load = field.extensions.printer.load('cursor')
						temp_buf = _temporary_buffer_name(condition_field_name)
						temp_buf_limit = f'{temp_name}.size()'
						body += '// deserialize to temporary buffer for further processing\n'
						body += f'final {java_type} {temp_name} = {temp_load};\n'
						body += f'final BufferView {temp_buf} = cursor.window({temp_buf_limit});\n'
						body += f'cursor.shiftRight({temp_buf_limit}); // skip temporary\n\n'
					queued_fields[condition_field_name].append(field)
					continue

			body += self.generate_deserialize_field(field)
			processed_fields.add(field.name)

			# process queued conditional fields once their condition field is read
			for queued_field in queued_fields.get(field.name, []):
				body += self.generate_deserialize_field(queued_field, _temporary_buffer_name(field.name))

		body += '\n'
		for field in self.non_reserved_fields(include_inherited=False):
			body += f'{self.field_name(field, "instance")} = {field.extensions.printer.name};\n'

		if not self.is_type_abstract:
			body += 'return instance;'

		return body

	def get_deserialize_descriptor(self):
		descriptor = MethodDescriptor(
			body=self._deserialize_body(),
			arguments=['java.nio.ByteBuffer buffer'] if not self.is_type_abstract
			else ['org.symbol.sdk.utils.BufferView cursor', f'{self.typename} instance'])
		descriptor.result = self.typename if not self.is_type_abstract else 'void'
		descriptor.is_static = True
		# abstract types expose `deserializeInto` (a BufferView-cursor field reader) to disambiguate from the concrete
		# `deserialize(ByteBuffer)`
		if self.is_type_abstract:
			descriptor.method_name = 'deserializeInto'
			descriptor.visibility = None
		return descriptor

	def _bound_size_value(self, field, condition):
		bound_field = field.extensions.bound_field
		bound_field_name = self.field_name(bound_field)
		field_value = ''
		if bound_field.display_type.is_array:
			if field.name.endswith('_count') or not bound_field.field_type.is_byte_constrained:
				# byte arrays expose `.length` (Java primitive byte[]); typed arrays expose `.size()` (List).
				if DisplayType.BYTE_ARRAY == bound_field.display_type:
					field_value = f'{bound_field_name}.length'
				else:
					field_value = f'{bound_field_name}.size()'

				bound_condition = self.generate_condition(bound_field, True)
				if condition and bound_condition:
					raise RuntimeError('do not know yet how to generate both conditions')

				if bound_condition:
					# the size field's Java type drives the sentinel literal's int/long form
					condition_value = _java_int_literal(bound_field.value.value, field.extensions.printer.is_long())
					field_value = f'(null != {bound_field_name} ? {field_value} : {condition_value})'
			else:
				field_value = bound_field.extensions.printer.get_size()
		elif field.is_size_reference:
			field_value = bound_field.extensions.printer.get_size()

		# field_value is always int (.size() / .length / get_size()); only a widening to long needs an explicit cast
		java_type = field.extensions.printer.get_type()
		if 'int' != java_type:
			field_value = f'({java_type}) ({field_value})'

		return field_value

	def generate_serialize_field(self, field):
		condition = self.generate_condition(field, True)

		if is_bound_size(field):
			comment = f' // bound: {field.name}'
			field_value = self._bound_size_value(field, condition)
		else:
			comment = ''
			field_value = self.field_name(field)

		direct_write = field.extensions.printer.serialize_into('buffer', field_value)
		if DisplayType.TYPED_ARRAY == field.display_type:
			serialize_field = field.extensions.printer.store(field_value, 'buffer')
			line = f'{serialize_field};{comment}\n'
		elif direct_write is not None:
			# Hot path for integer fields: writes go straight through the LE ByteBuffer instead
			# of allocating an intermediate byte[] via Converter.intToBytes.
			line = f'{direct_write};{comment}\n'
		else:
			serialize_field = field.extensions.printer.store(field_value)
			line = f'buffer.write({serialize_field});{comment}\n'

		return _indent_if_conditional(condition, line)

	def _generate_serialize_fields_body(self):
		body = ''
		fields_iter = self.non_const_fields(include_inherited=False)
		first_field = next(fields_iter, None)
		if first_field is None:
			return body

		if self.struct.size == first_field.extensions.printer.name:
			body += f'buffer.writeInt((long) this.size(), {first_field.size});\n'
		else:
			body += self.generate_serialize_field(first_field)

		for field in fields_iter:
			body += self.generate_serialize_field(field)

		return body

	def get_serialize_descriptor(self):
		return None

	def get_serialize_protected_descriptor(self):
		# Every struct (abstract or concrete) emits serializeInto. Concrete subclasses of an
		# abstract base chain to super.serializeInto(buffer) before writing their own fields.
		body = ''
		if self.base_struct:
			body += 'super.serializeInto(buffer);\n'

		body += self._generate_serialize_fields_body()
		descriptor = MethodDescriptor(
			method_name='serializeInto',
			arguments=['org.symbol.sdk.utils.Writer buffer'],
			body=body)
		descriptor.annotations = ['@Override']
		descriptor.visibility = 'protected'
		descriptor.result = 'void'
		return descriptor

	def _generate_size_field(self, field):
		condition = self.generate_condition(field, True)
		size_expr = field.extensions.printer.get_size()
		return _indent_if_conditional(condition, f'size += {size_expr};\n')

	def get_size_descriptor(self):
		body = 'int size = 0;\n'
		if self.base_struct:
			body += 'size += super.size();\n'

		body += ''.join(map(self._generate_size_field, self.non_const_fields(include_inherited=False)))
		body += 'return size;'
		descriptor = MethodDescriptor(body=body)
		descriptor.result = 'int'
		descriptor.annotations = ['@Override']  # overrides Serializer.size()
		return descriptor

	def _create_getter(self, field):
		java_type = field.extensions.printer.get_type()
		printer_name = field.extensions.printer.name

		# Computed fields are accessor-only with a special "Computed" suffix; they sidestep the
		# conditional/Optional handling because they're always derived (never null).
		if is_computed(field):
			method_name = f'get{capitalize(printer_name)}Computed'
			sizeref = field.field_type.sizeref
			java_field = lang_field_name(sizeref.property_name)
			body = f'return null != this.{java_field} ? this.{java_field}.size() + {sizeref.delta} : 0;'
			descriptor = MethodDescriptor(method_name=method_name, body=body)
			descriptor.result = java_type
			return descriptor

		method_name = f'get{capitalize(printer_name)}'
		if isinstance(field.extensions.printer, ArrayPrinter):
			# byte[] fields return the stored reference directly
			body = f'return {self.field_name(field)};'
			result = java_type
		elif field.is_conditional:
			# Conditional/union fields are genuinely nullable — return Optional<T> so the API
			# surfaces the presence/absence semantics instead of relying on a null sentinel.
			body = f'return java.util.Optional.ofNullable({self.field_name(field)});'
			result = f'java.util.Optional<{java_type}>'
		else:
			body = f'return {self.field_name(field)};'
			result = java_type

		descriptor = MethodDescriptor(method_name=method_name, body=body)
		descriptor.result = result
		return descriptor

	def _create_setter(self, field):
		java_type = field.extensions.printer.get_type()
		printer_name = field.extensions.printer.name
		method_name = f'set{capitalize(printer_name)}'
		descriptor = MethodDescriptor(
			method_name=method_name,
			arguments=[f'{java_type} value'],
			body=f'{self.field_name(field)} = value;')
		descriptor.result = 'void'
		return descriptor

	def get_getter_setter_descriptors(self):
		descriptors = []
		for field in self.non_reserved_fields(include_inherited=False):
			descriptors.append(self._create_getter(field))
			descriptors.append(self._create_setter(field))

		for field in self.computed_fields(include_inherited=False):
			descriptors.append(self._create_getter(field))

		return descriptors

	def get_extra_methods(self):
		"""Emit the reflection-free descriptor surface (typeHints/setField/getField)."""
		methods = []
		type_hints = self._render_type_hints_method()
		if type_hints is not None:
			methods.append(type_hints)

		methods.append(self._render_set_field_method())
		methods.append(self._render_get_field_method())

		# each NEM class is converting to non verifable form so no reflection is needed
		to_non_verifiable = self._render_to_non_verifiable_method()
		if to_non_verifiable is not None:
			methods.append(to_non_verifiable)

		return methods

	def _find_non_verifiable_sibling(self):
		"""Return the AST model for the {@code NonVerifiable<Self>} sibling, or {@code None}."""
		# Only the verifiable side carries a NonVerifiable sibling — skip the NonVerifiable
		# types themselves so we don't emit toNonVerifiable on those.
		if not self._ast_models or self.struct.name.startswith('NonVerifiable'):
			return None
		target_name = f'NonVerifiable{self.struct.name}'
		return next((candidate for candidate in self._ast_models if candidate.name == target_name), None)

	def _render_to_non_verifiable_method(self):
		"""Emit the typed {@code toNonVerifiable()} on NEM Transaction + its concrete subclasses."""
		sibling = self._find_non_verifiable_sibling()
		if sibling is None:
			return None

		if self.is_type_abstract:
			# Abstract Transaction → emit a concrete default that throws.
			descriptor = MethodDescriptor(
				method_name='toNonVerifiable',
				body='throw new IllegalStateException("toNonVerifiable() not implemented on " + getClass().getName());')
			descriptor.result = sibling.name
			descriptor.documentation = [
				'Returns the non-verifiable form of this transaction (omits the {@code signature}',
				'field). Used by the NEM facade for JSON payload preparation. Concrete subclasses',
				'override; calling this on the abstract base directly is a programmer error.',
				'', '@return Non-verifiable transaction.']
			return descriptor

		source_field_names = {field.extensions.printer.name for field in self.non_reserved_fields(include_inherited=True)}
		sibling_formatter = self._sibling_formatter(sibling)
		copy_lines = []
		for field in sibling_formatter.non_reserved_fields(include_inherited=True):
			name = field.extensions.printer.name
			if name in source_field_names:
				copy_lines.append(f'result.set{capitalize(name)}(this.{name});')

		copy_block = '\n'.join(copy_lines) + '\n' if copy_lines else ''
		descriptor = MethodDescriptor(
			method_name='toNonVerifiable',
			body=f'final {sibling.name} result = new {sibling.name}();\n{copy_block}return result;')
		descriptor.result = sibling.name
		descriptor.annotations = ['@Override']
		descriptor.documentation = ['Returns the non-verifiable form of this transaction.', '', '@return Non-verifiable transaction.']
		return descriptor

	def _sibling_formatter(self, sibling):
		factory_ast_model = None
		if sibling.factory_type:
			factory_ast_model = next(
				(candidate for candidate in self._ast_models if sibling.factory_type == candidate.name),
				None)

		return StructFormatter(sibling, factory_ast_model, factory_map=self.factory_map, ast_models=self._ast_models)

	def _render_type_hints_method(self):
		# pylint: disable=no-self-use
		descriptor = MethodDescriptor(method_name='typeHints', body='return TYPE_HINTS;')
		descriptor.result = 'java.util.Map<String, String>'
		descriptor.annotations = ['@Override']
		descriptor.documentation = [
			'Returns the type-hint map driving the rule-based descriptor pipeline.', '', '@return Field-name → rule-key map.']
		return descriptor

	def _render_field_switch(self, doc_lines, case_rhs, default_line, *, method_name, arguments, result, is_expression):
		"""Render a setField/getField descriptor — a name switch with one case per non-inherited field, always
		chaining to ``super`` so inherited fields land on the parent and unknown names ultimately throw."""
		# pylint: disable=too-many-arguments
		cases = [
			f'\tcase "{field.extensions.printer.name}" -> {case_rhs(field)};'
			for field in self.non_reserved_fields(include_inherited=False)]
		body_inner = '\n'.join(cases) + '\n' if cases else ''
		switch_open = 'return switch (name) {\n' if is_expression else 'switch (name) {\n'
		switch_close = '};' if is_expression else '}'
		descriptor = MethodDescriptor(
			method_name=method_name, arguments=arguments, body=f'{switch_open}{body_inner}\t{default_line}\n{switch_close}')
		descriptor.result = result
		descriptor.annotations = ['@Override']
		descriptor.documentation = doc_lines
		return descriptor

	def _render_set_field_method(self):
		"""Emit ``public void setField(String name, Object value)`` with a switch on field name."""
		return self._render_field_switch(
			['Sets a named field. Used by the rule-based descriptor pipeline.', '',
				'@param name  Field name (descriptor key).', '@param value Value to assign.'],
			lambda field: f'{self.field_name(field)} = {self._set_field_rhs(field)}',
			'default -> super.setField(name, value);',
			method_name='setField', arguments=['String name', 'Object value'], result=None,
			is_expression=False)

	def _render_get_field_method(self):
		"""Emit ``public Object getField(String name)`` mirroring the setField switch."""
		return self._render_field_switch(
			['Reads a named field. Used by the facade-level entity inspection helpers.', '',
				'@param name Field name.', '@return Field value.'],
			self.field_name,
			'default -> super.getField(name);',
			method_name='getField', arguments=['String name'], result='Object',
			is_expression=True)

	def _set_field_rhs(self, field):
		"""Pick the right convert helper / cast for a field's destination type."""
		# pylint: disable=no-self-use, too-many-return-statements
		printer = field.extensions.printer
		if isinstance(printer, ArrayPrinter):
			return 'asBytes(value)'

		if isinstance(printer, IntPrinter):
			size = printer.descriptor.size
			if 8 == size or (4 == size and printer.descriptor.is_unsigned):
				return 'asLong(value)'

			return 'asInt(value)'

		if isinstance(printer, BuiltinPrinter):
			display = printer.descriptor.display_type
			type_name = printer.descriptor.name
			if DisplayType.BYTE_ARRAY == display:
				# Generated POD classes and hand-written ByteArray subclasses both expose a byte[]-arg constructor — the function-target
				# binding for ``Function<byte[], T>`` disambiguates against any overloaded ctors.
				return f'asByteArray(value, {type_name}.class, {type_name}::new)'

			return f'({type_name}) value'

		if isinstance(printer, TypedArrayPrinter):
			# checked per-element coercion — avoids the unchecked (List<T>) cast
			return f'asList(value, {printer.descriptor.field_type.element_type}.class)'

		return f'({printer.get_type()}) value'

	def _generate_str_field(self, field):
		condition = self.generate_condition(field, True)
		field_to_string = field.extensions.printer.to_string(self.field_name(field))
		if DisplayType.TYPED_ARRAY == field.display_type:
			value = f'"[" + ({field_to_string}) + "]"'
		elif DisplayType.BYTE_ARRAY == field.display_type:
			value = f'"hex(" + ({field_to_string}) + ")"'
		else:
			value = field_to_string

		return _indent_if_conditional(
			condition,
			f'result.append("{field.extensions.printer.name}: ").append({value}).append(", ");\n')

	def get_str_descriptor(self):
		body = 'final StringBuilder result = new StringBuilder("(");\n'
		if self.base_struct:
			body += 'result.append(super.toString());\n'
		body += ''.join(map(self._generate_str_field, self.non_reserved_fields(include_inherited=False)))
		body += 'result.append(")");\n'
		body += 'return result.toString();'
		descriptor = MethodDescriptor(body=body)
		descriptor.annotations = ['@Override']
		descriptor.result = 'String'
		return descriptor

	def _generate_json_field(self, field):
		condition = self.generate_condition(field, True)
		field_to_json = field.extensions.printer.to_json(self.field_name(field))
		return _indent_if_conditional(
			condition,
			f'result.put("{field.extensions.printer.name}", {field_to_json});\n')

	def get_json_descriptor(self):
		# Build an insertion-ordered Map; subclasses merge their abstract parent's projection first.
		body = 'final java.util.Map<String, Object> result = new java.util.LinkedHashMap<>();\n'
		if self.base_struct:
			body += 'result.putAll(super.toJson());\n'

		body += ''.join(map(self._generate_json_field, self.non_reserved_fields(include_inherited=False)))
		body += 'return result;'
		descriptor = MethodDescriptor(body=body)
		descriptor.annotations = ['@Override']  # overrides the abstract CatbufferType.toJson() (or a parent struct's)
		descriptor.result = 'java.util.Map<String, Object>'
		return descriptor
