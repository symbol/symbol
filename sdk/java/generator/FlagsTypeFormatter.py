"""Emits a Java *class* (not enum) for each bitwise catbuffer enum AST model. Java's built-in `enum` cannot represent
OR-combined flag values, so a final class of named int/long flag constants with bitwise helpers is emitted instead.
"""

from .AbstractEnumFormatter import AbstractEnumFormatter
from .AbstractTypeFormatter import MethodDescriptor


class FlagsTypeFormatter(AbstractEnumFormatter):
	def get_implemented_interfaces(self):
		return 'org.symbol.sdk.Serializer'

	def get_fields(self):
		fields = []
		# public static final constant per named flag (including the conventional NONE if present)
		for entry in self.enum_type.values:
			fields.append(
				f'public static final {self.typename} {entry.name} = new {self.typename}({self._value_literal(entry.value)});')

		fields.append('')
		fields.append(f'public final {self.value_type} value;')
		return fields

	def get_ctor_descriptor(self):
		return MethodDescriptor(body='this.value = value;', arguments=[f'{self.value_type} value'])

	def _deserialize_body(self):
		body = f'final {self.value_type} value = {self.int_printer.load(self._wrapped_cursor())};\n'
		body += f'return new {self.typename}(value);'
		return body

	def get_getter_setter_descriptors(self):
		methods = [self._get_value_descriptor()]

		descriptor = MethodDescriptor(
			method_name='fromValue',
			arguments=[f'{self.value_type} value'],
			body=f'return new {self.typename}(value);')
		descriptor.result = self.typename
		descriptor.is_static = True
		methods.append(descriptor)

		methods.extend(self._parse_descriptors())

		descriptor = MethodDescriptor(
			method_name='has',
			arguments=[f'{self.value_type} flag'],
			body=self._has_flag_body())
		descriptor.result = 'boolean'
		methods.append(descriptor)

		descriptor = MethodDescriptor(
			method_name='equals',
			arguments=['Object other'],
			body=(
				f'if (this == other)\n'
				f'\treturn true;\n'
				f'\n'
				f'if (!(other instanceof {self.typename}))\n'
				f'\treturn false;\n'
				f'\n'
				f'return this.value == (({self.typename}) other).value;'
			))
		descriptor.annotations = ['@Override']
		descriptor.result = 'boolean'
		methods.append(descriptor)

		descriptor = MethodDescriptor(method_name='hashCode', body=self._hashcode_body())
		descriptor.annotations = ['@Override']
		descriptor.result = 'int'
		methods.append(descriptor)
		return methods

	def _parse_descriptors(self):
		"""``parse(Object)`` + ``fromName(String)`` — non-reflective descriptor coercion (instance /
		space-separated flag names / number), registered via ``Models.FACTORIES``."""
		zero = '0L' if 'long' == self.value_type else '0'
		string_branch = (
			f'if (rawValue instanceof String names) {{\n'
			f'\t{self.value_type} combined = {zero};\n'
			f'\tfor (final String name : names.split(" "))\n'
			f'\t\tcombined |= fromName(name).value;\n'
			f'\n'
			f'\treturn new {self.typename}(combined);\n'
			f'}}'
		)
		parse = self._build_parse_descriptor(string_branch, f'new {self.typename}({self._number_expression()})')

		name_cases = '\n'.join(
			f'\tcase "{entry.name.lower()}" -> {entry.name};' for entry in self.enum_type.values)
		from_name_body = (
			f'return switch (name.toLowerCase(java.util.Locale.ROOT)) {{\n'
			f'{name_cases}\n'
			f'\tdefault -> throw new IllegalArgumentException('
			f'"unknown value " + name + " for type {self.typename}");\n'
			f'}};'
		)
		from_name = MethodDescriptor(method_name='fromName', arguments=['String name'], body=from_name_body)
		from_name.result = self.typename
		from_name.is_static = True
		from_name.visibility = 'private'
		return [parse, from_name]

	def _hashcode_body(self):
		if 'long' == self.value_type:
			return 'return Long.hashCode(this.value);'

		return 'return Integer.hashCode(this.value);'

	def get_str_descriptor(self):
		values = ', '.join(self._value_literal(entry.value) for entry in self.enum_type.values)
		names = ', '.join(f'"{self.typename}.{entry.name}"' for entry in self.enum_type.values)
		body = (
			f'final {self.value_type}[] values = {{ {values} }};\n'
			f'final String[] names = {{ {names} }};\n'
			f'final java.util.List<String> parts = new java.util.ArrayList<>();\n'
			f'for (int i = 0; i < values.length; ++i) {{\n'
			f'\tif (values[i] == this.value)\n'
			f'\t\treturn names[i];\n'
			f'\tif (this.has(values[i]))\n'
			f'\t\tparts.add(names[i]);\n'
			f'}}\n'
			f'\n'
			f'return String.join("|", parts);'
		)
		return AbstractEnumFormatter.make_to_string_descriptor(body)
