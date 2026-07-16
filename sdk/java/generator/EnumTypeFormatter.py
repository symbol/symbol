"""Emits a Java ``enum`` for each (non-bitwise) catbuffer enum AST model, with a ``value()`` accessor and a
``fromValue(...)`` lookup. Bitwise enums are handled separately by :mod:`FlagsTypeFormatter`.
"""

from .AbstractEnumFormatter import AbstractEnumFormatter
from .AbstractTypeFormatter import MethodDescriptor


class EnumTypeFormatter(AbstractEnumFormatter):
	def get_static_block_lines(self):
		"""Emit a private static ``VALUE_MAP`` and initializer block for ``fromValue`` lookup."""
		boxed = self._boxed_value_type()
		return [
			f'private static final java.util.Map<{boxed}, {self.typename}> VALUE_MAP;',
			'static {',
			f'\tfinal java.util.Map<{boxed}, {self.typename}> map = new java.util.HashMap<>();',
			f'\tfor ({self.typename} candidate : values())',
			'\t\tmap.put(candidate.value, candidate);',
			'\tVALUE_MAP = java.util.Collections.unmodifiableMap(map);',
			'}',
		]

	def _boxed_value_type(self):
		return 'Integer' if 'int' == self.value_type else 'Long'

	def get_enum_constants(self):
		"""Return the ``CONST(value),`` lines that go at the top of the enum body."""
		entries = []

		for entry in self.enum_type.values:
			entries.append(f'{entry.name}({self._value_literal(entry.value)})')

		return entries

	def get_ctor_descriptor(self):
		descriptor = MethodDescriptor(
			body='this.value = value;',
			arguments=[f'{self.value_type} value'])
		descriptor.visibility = 'private'
		return descriptor

	def _deserialize_body(self):
		body = f'final {self.value_type} value = {self.int_printer.load(self._wrapped_cursor())};\n'
		body += 'return fromValue(value);'
		return body

	def get_serialize_descriptor(self):
		descriptor = MethodDescriptor(body=f'return {self.int_printer.store("this.value")};')
		descriptor.result = 'byte[]'
		descriptor.annotations = ['@Override']  # overrides Serializer.serialize()
		return descriptor

	def get_size_descriptor(self):
		descriptor = MethodDescriptor(body=f'return {self.enum_type.size};')
		descriptor.result = 'int'
		descriptor.annotations = ['@Override']  # overrides Serializer.size()
		return descriptor

	def get_getter_setter_descriptors(self):
		methods = [self._get_value_descriptor()]

		# fromValue(int/long) — O(1) lookup against the static VALUE_MAP.
		body = (
			f'final {self.typename} result = VALUE_MAP.get(value);\n'
			f'if (null == result)\n'
			f'\tthrow new IllegalArgumentException("invalid enum value " + value);\n'
			f'return result;'
		)
		descriptor = MethodDescriptor(method_name='fromValue', arguments=[f'{self.value_type} value'], body=body)
		descriptor.result = self.typename
		descriptor.is_static = True
		methods.append(descriptor)

		# parse(Object) — non-reflective coercion (instance / constant name / number), registered via
		# Models.FACTORIES for the rule-based factory but also callable directly as a general constructor.
		string_branch = (
			f'if (rawValue instanceof String name) {{\n'
			f'\ttry {{\n'
			f'\t\treturn valueOf(name.toUpperCase(java.util.Locale.ROOT));\n'
			f'\t}} catch (final IllegalArgumentException ex) {{\n'
			f'\t\tthrow new IllegalArgumentException('
			f'"unknown value " + name + " for type {self.typename}", ex);\n'
			f'\t}}\n'
			f'}}'
		)
		methods.append(self._build_parse_descriptor(string_branch, f'fromValue({self._number_expression()})'))

		return methods

	def get_str_descriptor(self):
		# default Java enum.toString returns the constant name; we wrap as "EnumName.CONSTANT"
		return AbstractEnumFormatter.make_to_string_descriptor(f'return "{self.typename}." + name();')
