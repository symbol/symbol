"""Per-type Java printers — produces Java source fragments for every leaf field shape.
Each :class:`Printer` subclass knows how to render, for one field of one shape.
"""

from catparser.DisplayType import DisplayType

from .name_formatting import capitalize, lang_field_name, underline_name


def _java_bool(value):
	return 'true' if value else 'false'


DEFAULT_CURSOR = 'view'


class Printer:
	"""Base class — owns the resolved Java field name."""

	def __init__(self, descriptor, name):
		self.descriptor = descriptor
		# printer.name is the 'fixed' field name (camelCase, post Java keyword-collision rewrite)
		self.name = lang_field_name(name or underline_name(self.descriptor.name))

	def sort(self, _field_name):  # pylint: disable=no-self-use
		return None

	def serialize_into(self, _buffer_name, _field_name):  # pylint: disable=no-self-use
		"""Direct-write hook used by struct serializers."""
		return None


class IntPrinter(Printer):
	"""Fixed-width integer field. Java backing type is chosen by size + signedness:
	* size 1, 2: ``int`` (Java has no smaller integer literals; ``int`` covers both).
	* size 4: ``int`` when signed or an array/byte-array bound (always fits a positive int); ``long`` for an exposed u32.
	* size 8: ``long`` (any u64).
	"""

	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = None

	def _is_bound_size(self):
		# size / count fields that bound an array or byte-array are internal deserialize temporaries whose value always
		# fits in a int.
		extensions = getattr(self.descriptor, 'extensions', None)
		return extensions is not None and getattr(extensions, 'bound_field', None) is not None

	def is_long(self):
		"""Java backing type is long iff the value cannot fit a signed int: any u64, or an exposed (non-bound) u32."""
		size = self.descriptor.size
		if 8 == size:
			return True

		return 4 == size and self.descriptor.is_unsigned and not self._is_bound_size()

	def get_type(self):
		return 'long' if self.is_long() else 'int'

	def get_default_value(self):
		return '0L' if self.is_long() else '0'

	def get_size(self):
		return self.descriptor.size

	def load(self, cursor=DEFAULT_CURSOR):
		size = self.descriptor.size
		signed = _java_bool(not self.descriptor.is_unsigned)
		# Non-advancing read at the cursor's position (the caller shiftRights by the field's size).
		expr = f'{cursor}.peekInt({size}, {signed})'

		# long-backed values pass through; everything else (1/2-byte, 4-byte signed, and bound u32 size/count) narrows to int
		if self.is_long():
			return expr

		return f'(int) ({expr})'

	def advancement_size(self):
		return self.get_size()

	def store(self, field_name):
		size = self.descriptor.size
		return f'org.symbol.sdk.utils.Converter.intToBytes({field_name}, {size})'

	def serialize_into(self, buffer_name, field_name):
		"""Direct-write into a :class:`Writer`'s underlying ByteBuffer."""
		size = self.descriptor.size
		return f'{buffer_name}.writeInt({field_name}, {size})'

	@staticmethod
	def assign(value):
		return str(value)

	def to_string(self, field_name):
		# format as "0x" + uppercase hex
		if 8 == self.descriptor.size:
			# Long.toHexString renders the unsigned 64-bit bit pattern
			return f'"0x" + Long.toHexString({field_name}).toUpperCase()'

		# 4-byte: render via Long to avoid int sign-extension; the signed case is masked to 32 bits
		if 4 == self.descriptor.size:
			value = field_name if self.descriptor.is_unsigned else f'((long) {field_name}) & 0xFFFFFFFFL'
			return f'"0x" + Long.toHexString({value}).toUpperCase()'

		# 1/2-byte: mask to suppress sign extension
		mask = {1: '0xFF', 2: '0xFFFF'}[self.descriptor.size]
		return f'"0x" + Integer.toHexString({field_name} & {mask}).toUpperCase()'

	def to_json(self, field_name):
		# <=32-bit raw ints are emitted as numbers; 64-bit values are stringified (JSON has no
		# native 64-bit integer), matching BaseValue.toJson().
		if 8 == self.descriptor.size:
			if self.descriptor.is_unsigned:
				return f'Long.toUnsignedString({field_name})'

			return f'Long.toString({field_name})'

		return field_name


class ArrayPrinter(Printer):
	"""Fixed-or-variable size byte array. Java backing type is always ``byte[]``."""

	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = 'bytes_array'

	@staticmethod
	def get_type():
		return 'byte[]'

	def get_default_value(self):
		size = self.descriptor.size
		if isinstance(size, str):
			return 'new byte[0]'

		return f'new byte[{size}]'

	def get_size(self):
		size = self.descriptor.size
		if isinstance(size, str):
			return f'this.{self.name}.length'

		return size

	def load(self, cursor=DEFAULT_CURSOR):
		# Copy exactly 'advancement_size' bytes, bounded to the cursor's window (a corrupt size local throws instead of
		# reading into the following sibling's bytes)
		size = self.advancement_size()
		return f'{cursor}.peekBytes({size})'

	def advancement_size(self):
		size = self.descriptor.size
		if not isinstance(size, str):
			return size

		# variable-size: the size lives in another local of the same name (camelCase)
		return lang_field_name(size)

	@staticmethod
	def store(field_name):
		# byte[] is written verbatim into the Writer
		return field_name

	@staticmethod
	def to_string(field_name):
		return f'org.symbol.sdk.utils.Converter.uint8ToHex({field_name})'

	@staticmethod
	def to_json(field_name):
		# byte arrays are JSON-encoded as a hex string
		return f'org.symbol.sdk.utils.Converter.uint8ToHex({field_name})'


class TypedArrayPrinter(Printer):
	"""Homogeneous array of generated types -> ``java.util.List<T>``."""

	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = f'array[{self.descriptor.field_type.element_type}]'

	def get_type(self):
		return f'java.util.List<{self.descriptor.field_type.element_type}>'

	@staticmethod
	def get_default_value():
		return 'new java.util.ArrayList<>()'

	@property
	def is_variable_size(self):
		descriptor = self.descriptor
		is_constrained = descriptor.field_type.is_byte_constrained or descriptor.extensions.is_contents_abstract
		return is_constrained and descriptor.field_type.alignment

	@property
	def _skip_last_padding(self):
		"""Java boolean literal for ArrayHelpers' skip-last-element-padding argument."""
		return _java_bool(not self.descriptor.field_type.is_last_element_padded)

	def get_size(self):
		"""Java expression that yields the serialized byte size of this list (instance form)."""
		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip = self._skip_last_padding
			return f'org.symbol.sdk.utils.ArrayHelpers.size(this.{self.name}, {alignment}, {skip})'

		return f'org.symbol.sdk.utils.ArrayHelpers.size(this.{self.name})'

	def load(self, cursor=DEFAULT_CURSOR):
		element_type = self.descriptor.field_type.element_type
		# Abstract element types dispatch through XFactory.deserialize
		if self.descriptor.extensions.is_contents_abstract:
			factory = f'{element_type}Factory::deserialize'
		else:
			factory = f'{element_type}::deserialize'

		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip = self._skip_last_padding
			if self.descriptor.field_type.is_expandable:
				return (
					f'org.symbol.sdk.utils.ArrayHelpers.readVariableSizeElements('
					f'{cursor}, {factory}, {alignment}, {skip})')

			data_size = lang_field_name(self.descriptor.size)
			# bound the read to the declared byte size (zero-copy window)
			return (
				f'org.symbol.sdk.utils.ArrayHelpers.readVariableSizeElements('
				f'{cursor}.window({data_size}), {factory}, {alignment}, {skip})')

		comparator = self.sort_comparator()
		comparator_argument = f', {comparator}' if comparator else ''
		if self.descriptor.field_type.is_expandable:
			return f'org.symbol.sdk.utils.ArrayHelpers.readArray({cursor}, {factory}{comparator_argument})'

		count_expr = lang_field_name(str(self.descriptor.size))
		# the count field is a bound size, hence int (see IntPrinter.is_long) — used directly, no narrowing cast
		return (
			f'org.symbol.sdk.utils.ArrayHelpers.readArrayCount('
			f'{cursor}, {factory}, {count_expr}{comparator_argument})')

	def advancement_size(self):
		"""Local-variable form of get_size (no ``this.`` prefix)."""
		if self.descriptor.field_type.is_byte_constrained:
			return lang_field_name(str(self.descriptor.size))

		alignment = self.descriptor.field_type.alignment
		if alignment:
			skip = self._skip_last_padding
			return f'org.symbol.sdk.utils.ArrayHelpers.size({self.name}, {alignment}, {skip})'

		return f'org.symbol.sdk.utils.ArrayHelpers.size({self.name})'

	def store(self, field_name, buffer_name):
		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip = self._skip_last_padding
			return f'org.symbol.sdk.utils.ArrayHelpers.writeVariableSizeElements({buffer_name}, {field_name}, {alignment}, {skip})'

		comparator = self.sort_comparator()
		comparator_argument = f', {comparator}' if comparator else ''
		if self.descriptor.field_type.is_expandable:
			return f'org.symbol.sdk.utils.ArrayHelpers.writeArray({buffer_name}, {field_name}{comparator_argument})'

		count_expr = self.descriptor.size
		if isinstance(count_expr, str):
			# size lives in another field (rare for fixed-count arrays)
			return f'org.symbol.sdk.utils.ArrayHelpers.writeArray({buffer_name}, {field_name}{comparator_argument})'

		return f'org.symbol.sdk.utils.ArrayHelpers.writeArrayCount({buffer_name}, {field_name}, {count_expr}{comparator_argument})'

	def sort_comparator(self):
		"""Sort-key comparator expression for this array, or None when the schema declares no sort key."""
		if not self.descriptor.field_type.sort_key:
			return None
		sort_key = lang_field_name(self.descriptor.field_type.sort_key)
		getter = f'get{capitalize(sort_key)}'
		element_type = self.descriptor.field_type.element_type
		return f'java.util.Comparator.comparing({element_type}::{getter})'

	def sort(self, field_name):
		comparator = self.sort_comparator()
		if not comparator:
			return None

		return f'{field_name}.sort({comparator});'

	@staticmethod
	def to_string(field_name):
		return f'{field_name}.stream().map(Object::toString).collect(java.util.stream.Collectors.joining(","))'

	@staticmethod
	def to_json(field_name):
		# collect each element's own toJson into a List
		return (
			f'{field_name}.stream().map(e -> (Object) e.toJson())'
			f'.collect(java.util.stream.Collectors.toList())')


class BuiltinPrinter(Printer):
	"""Reference to another generated type (POD int, POD byte-array, enum, struct)."""

	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = {
			DisplayType.INTEGER: 'pod:',
			DisplayType.BYTE_ARRAY: 'pod:',
			DisplayType.TYPED_ARRAY: 'pod:',
			DisplayType.ENUM: 'enum:',
			DisplayType.STRUCT: 'struct:',
		}[self.descriptor.display_type]
		self.type_hint += self.descriptor.name

	def get_type(self):
		return self.descriptor.name

	def get_default_value(self):
		if DisplayType.ENUM == self.descriptor.display_type:
			first_enum_value_name = self.descriptor.values[0].name
			return f'{self.get_type()}.{first_enum_value_name}'

		return f'new {self.get_type()}()'

	def get_size(self):
		return f'this.{self.name}.size()'

	def load(self, cursor=DEFAULT_CURSOR):
		display_type = self.descriptor.display_type
		# Abstract structs route through XFactory; the special 'parent' name is handled by the
		# Factory class itself, so we leave that detail to FactoryFormatter.
		if DisplayType.STRUCT == display_type and self.descriptor.is_abstract:
			if 'parent' != self.name:
				return f'{self.get_type()}Factory.deserialize({cursor})'

		return f'{self.get_type()}.deserialize({cursor})'

	def advancement_size(self):
		return f'{self.name}.size()'

	@staticmethod
	def store(field_name):
		return f'{field_name}.serialize()'

	def sort(self, field_name):
		# only structs need an internal sort step (typed-array sorts are handled by their printer)
		return f'{field_name}.sort();' if DisplayType.STRUCT == self.descriptor.display_type else None

	def assign(self, value):
		return f'{self.get_type()}.{value}'

	@staticmethod
	def to_string(field_name):
		return f'{field_name}.toString()'

	@staticmethod
	def to_json(field_name):
		# Delegate to the referenced type's own toJson (POD -> String/number, enum/flags -> numeric
		# value, struct -> Map<String, Object>).
		return f'{field_name}.toJson()'


def create_pod_printer(descriptor, name=None):
	"""Pick the printer subclass for a POD descriptor (used by `extend_models`)."""
	display_type = descriptor.display_type
	if DisplayType.INTEGER == display_type:
		printer_cls = IntPrinter
	elif DisplayType.BYTE_ARRAY == display_type:
		printer_cls = ArrayPrinter
	else:
		printer_cls = TypedArrayPrinter

	return printer_cls(descriptor, name)
