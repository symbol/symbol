"""Shared base for the enum and bitwise-flags formatters."""

from abc import abstractmethod

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import IntPrinter


class AbstractEnumFormatter(AbstractTypeFormatter):
	def __init__(self, ast_model):
		super().__init__()
		self.enum_type = ast_model
		self.int_printer = IntPrinter(self.enum_type)

	@property
	def typename(self):
		return self.enum_type.name

	@property
	def value_type(self):
		"""Java type of the underlying integer."""
		return self.int_printer.get_type()

	def _value_literal(self, value):
		"""Render an integer literal in the right Java type for this type's backing value."""
		if 'long' == self.value_type:
			return f'{value}L'

		return str(value)

	@abstractmethod
	def _deserialize_body(self):
		raise NotImplementedError('subclasses emit their own deserialize body')

	def get_deserialize_descriptor(self):
		descriptor = MethodDescriptor(body=self._deserialize_body(), arguments=['java.nio.ByteBuffer buffer'])
		descriptor.result = self.typename
		descriptor.is_static = True
		return descriptor

	def get_size_descriptor(self):
		descriptor = MethodDescriptor(body=f'return {self.enum_type.size};')
		descriptor.result = 'int'
		descriptor.annotations = ['@Override']  # overrides Serializer.size()
		return descriptor

	def get_serialize_descriptor(self):
		descriptor = MethodDescriptor(body=f'return {self.int_printer.store("this.value")};')
		descriptor.result = 'byte[]'
		descriptor.annotations = ['@Override']  # overrides Serializer.serialize()
		return descriptor

	@staticmethod
	def _wrapped_cursor():
		"""BufferView over the incoming ByteBuffer ``buffer`` param — the cursor enum/flags read their value from."""
		return 'new org.symbol.sdk.utils.BufferView(buffer)'

	def _get_value_descriptor(self):
		"""``getValue()`` accessor exposing the underlying integer value."""
		descriptor = MethodDescriptor(method_name='getValue', body='return this.value;')
		descriptor.result = self.value_type
		return descriptor

	@staticmethod
	def make_to_string_descriptor(body):
		"""Wrap a ``toString`` body into an overriding String-typed method descriptor."""
		descriptor = MethodDescriptor(body=body)
		descriptor.annotations = ['@Override']
		descriptor.result = 'String'
		return descriptor

	def get_json_descriptor(self):
		# JSON projection is the underlying numeric value
		if 8 == self.enum_type.size:
			render = 'toUnsignedString' if self.enum_type.is_unsigned else 'toString'
			descriptor = MethodDescriptor(body=f'return Long.{render}(this.value);')
			descriptor.result = 'String'
			return descriptor

		descriptor = MethodDescriptor(body='return this.value;')
		descriptor.result = self.value_type
		return descriptor

	@staticmethod
	def _has_flag_body():
		"""Body of the bitwise ``has(flag)`` check emitted for flags types."""
		return 'return 0 != (this.value & flag);'

	def _number_expression(self):
		"""``Converter.toInt/toLong(number)`` matching the backing value type."""
		converter = 'toLong' if 'long' == self.value_type else 'toInt'
		return f'org.symbol.sdk.utils.Converter.{converter}(number)'

	def _build_parse_descriptor(self, string_branch, number_result):
		"""Assemble the shared ``parse(Object)`` skeleton (instance / String / Number / reject); the subclass
		supplies the String-branch block and the ``Number`` result construction."""
		body = (
			f'if (rawValue instanceof {self.typename} typed)\n'
			f'\treturn typed;\n'
			f'{string_branch}\n'
			f'if (rawValue instanceof Number number)\n'
			f'\treturn {number_result};\n'
			f'throw new IllegalArgumentException(\n'
			f'\t\t"cannot parse " + rawValue.getClass().getName() + " into {self.typename}");'
		)
		descriptor = MethodDescriptor(method_name='parse', arguments=['Object rawValue'], body=body)
		descriptor.result = self.typename
		descriptor.is_static = True
		return descriptor
