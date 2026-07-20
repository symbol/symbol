"""Emits a Java POD class for each catbuffer ``byte_array`` or ``integer`` AST model."""

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import create_pod_printer


class PodTypeFormatter(AbstractTypeFormatter):
	def __init__(self, ast_model):
		super().__init__()
		self.pod = ast_model
		self.printer = create_pod_printer(self.pod)

	@property
	def typename(self):
		return self.pod.name

	@property
	def _is_array(self):
		return self.pod.display_type.is_array

	@property
	def _is_signed(self):
		return not self.pod.is_unsigned if not self._is_array else False

	def get_base_class(self):
		if self._is_array:
			return 'org.symbol.sdk.ByteArray'

		return f'org.symbol.sdk.BaseValue<{self.typename}>'

	def get_implemented_interfaces(self):
		# PODs reach Serializer through their base class; there is nothing extra to implement.
		return None

	def get_fields(self):
		lines = [f'public static final int SIZE = {self.pod.size};']
		if not self._is_array:
			lines.append(f'public static final boolean IS_SIGNED = {str(self._is_signed).lower()};')

		return lines

	def get_ctor_descriptor(self):
		# Canonical constructor delegates to the base class, which validates and stores the backing value.
		if self._is_array:
			return MethodDescriptor(body='super(bytes, SIZE);', arguments=['byte[] bytes'])

		return MethodDescriptor(body='super(value, SIZE, IS_SIGNED);', arguments=['long value'])

	def get_extra_methods(self):
		"""No-arg ctor + descriptor parse factory; equals/hashCode/toString are inherited from the base class."""
		typename = self.typename

		# Integer PODs take a long; the canonical long ctor already covers the natural-type case (no extra ctor needed).
		default = self.printer.get_default_value() if self._is_array else '0L'
		ctor = MethodDescriptor(body=f'this({default});')
		ctor.is_constructor = True

		# Raw-value factory — the non-reflective coercion used by the rule-based factory (registered via
		# Models.POD_FACTORIES), but also a general-purpose constructor callable directly. Each pod knows its own input shapes.
		typed_branch = (
			f'if (rawValue instanceof {typename} typed)\n'
			f'\treturn typed;\n')
		if self._is_array:
			# the hex-string / byte[] / ByteArray coercion is centralized in ByteArray.toBytes (the byte-array
			# counterpart of Converter.toLong); a String is read there as hex (this type's canonical form).
			parse_doc = f'Parses a raw value ({typename}, hex string, byte array, or SDK ByteArray) into a {typename}.'
			parse_body = (
				f'{typed_branch}'
				f'return new {typename}(org.symbol.sdk.ByteArray.toBytes(rawValue));')
		else:
			parse_doc = f'Parses a raw value ({typename}, number, or decimal/0x-hex string) into a {typename}.'
			parse_body = (
				f'{typed_branch}'
				'if (rawValue instanceof String string)\n'
				f'\treturn new {typename}(org.symbol.sdk.utils.Converter.toLong(string));\n'
				'if (rawValue instanceof Number number)\n'
				f'\treturn new {typename}(org.symbol.sdk.utils.Converter.toLong(number));\n'
				'throw new IllegalArgumentException(\n'
				'\t\t"cannot convert " + (null == rawValue ? "null" : rawValue.getClass().getName())'
				' + " to an integer value");')

		parse = MethodDescriptor(method_name='parse', arguments=['Object rawValue'], body=parse_body)
		parse.is_static = True
		parse.result = typename
		parse.documentation = [parse_doc, '', '@param rawValue Raw value.', '@return Parsed value.']

		# equals/hashCode/toString/serialize/size are all inherited from the POD's base class.
		return [ctor, parse]

	def get_deserialize_descriptor(self):
		view = 'new org.symbol.sdk.utils.BufferView(buffer)'
		if self._is_array:
			body = f'return new {self.typename}({view}.peekBytes(SIZE));'
		else:
			body = f'return new {self.typename}({view}.peekInt(SIZE, IS_SIGNED));'

		descriptor = MethodDescriptor(body=body, arguments=['java.nio.ByteBuffer buffer'])
		descriptor.result = self.typename
		descriptor.is_static = True
		return descriptor

	def get_serialize_descriptor(self):
		# Byte-array PODs use ByteArray's default serialize(); integer PODs inherit serialize() from BaseValue.
		return None

	def get_size_descriptor(self):
		# Byte-array PODs use ByteArray's default size() (bytes.length); integer PODs inherit size() from BaseValue.
		return None

	def get_str_descriptor(self):
		# Both POD kinds inherit toString() from their base class (hex; the hand-written Address overrides with base32).
		return None
