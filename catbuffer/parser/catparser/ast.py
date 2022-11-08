import re
from abc import ABC, abstractmethod
from collections import namedtuple

from lark import Token

from .DisplayType import DisplayType


class AstException(Exception):
	"""Exception raised when an AST violation is detected"""


# region utils (private)

def _get_token_value(token):
	return token.value if isinstance(token, Token) else token


def _set_if(source, type_descriptor, property_name):
	value = getattr(source, property_name)
	if value is not None:
		type_descriptor[property_name] = value


def _lookup_attribute_value(attributes, name, multi_value=False):
	if not attributes:
		return None

	attribute = next((attribute for attribute in attributes if attribute.name == name), None)
	if not attribute:
		return None

	return attribute.values if multi_value else attribute.value


def _format_attributes(attributes):
	if not attributes:
		return ''

	def format_attribute(attribute):
		if 'comparer' != attribute.name:
			return str(attribute)

		formatted_values = []
		for i in range(0, len(attribute.values) // 2):
			formatted_values.append(attribute.values[2 * i])
			if attribute.values[2 * i + 1]:
				formatted_values[-1] += f'!{attribute.values[2 * i + 1]}'

		return f'@{attribute.name}({", ".join(formatted_values)})'

	return '\n'.join(format_attribute(attribute) for attribute in attributes) + '\n'


def _format_is_unsigned(is_unsigned):
	return 'unsigned' if is_unsigned else 'signed'


# endregion

# region Statement

class Statement(ABC):
	"""Base class that allows comments to be attached to a top-level or sub-level declaration."""

	def __init__(self):
		self.comment = None

	@abstractmethod
	def _to_legacy_descriptor(self):
		pass

	def to_legacy_descriptor(self):
		"""Produces a dictionary consistent with the original catbuffer type descriptors."""

		if not self.comment:
			return self._to_legacy_descriptor()

		return {'comments': self.comment.parsed, **self._to_legacy_descriptor()}


# endregion

# region Comment

class Comment:
	"""Single or multiline comment."""

	def __init__(self, string):
		self.parsed = ''

		needs_separator = False
		for comment_line in string.split('\n'):
			comment_line = comment_line.strip('# \t')  # strip '#' and whitespace

			if not comment_line:
				self.parsed += '\n'
				needs_separator = False
			else:
				if needs_separator:
					self.parsed += ' '

				self.parsed += comment_line
				needs_separator = True

	def __str__(self):
		return self.parsed


# endregion

# region FixedSizeInteger

class FixedSizeInteger:
	"""Signed or unsigned integer type composed of 1, 2, 4 or 8 bytes."""

	SizeRef = namedtuple('SizeRef', ['property_name', 'delta'])

	def __init__(self, string):
		self.short_name = string

		self.is_unsigned = 'u' == string[0]
		self.size = int(string[3 + (1 if self.is_unsigned else 0):]) // 8

		self.display_type = DisplayType.INTEGER

		self._sizeref = None

	@property
	def name(self):
		"""Gets the type name."""
		return str(self)

	@property
	def sizeref(self):
		"""Gets the size reference."""
		return self._sizeref

	@sizeref.setter
	def sizeref(self, values):
		"""Sets the size reference."""
		self._sizeref = self.SizeRef(values[0], values[1])

	def copy(self, prefix):
		"""Creates a copy of this field and transforms field names using the specified prefix."""

		copy = FixedSizeInteger(self.short_name)
		if self.sizeref:
			copy.sizeref = [f'{prefix}_{self.sizeref.property_name}', self.sizeref.delta]  # pylint: disable=no-member

		return copy

	def to_legacy_descriptor(self):
		"""Produces a dictionary consistent with the original catbuffer type descriptors."""

		descriptor = {'size': self.size, 'type': 'byte', 'signedness': _format_is_unsigned(self.is_unsigned)}
		if self.sizeref:
			descriptor['sizeref'] = {
				'property_name': self.sizeref.property_name,  # pylint: disable=no-member
				'delta': self.sizeref.delta  # pylint: disable=no-member
			}

		return descriptor

	def __str__(self):
		return self.short_name


# endregion

# region FixedSizeBuffer

class FixedSizeBuffer:
	"""Fixed size buffer composed of a constant number of bytes."""

	def __init__(self, size):
		self.size = size
		self.is_unsigned = True

		self.display_type = DisplayType.BYTE_ARRAY

	@property
	def name(self):
		"""Gets the type name."""
		return str(self)

	def to_legacy_descriptor(self):
		"""Produces a dictionary consistent with the original catbuffer type descriptors."""

		return {'size': self.size, 'type': 'byte', 'signedness': _format_is_unsigned(self.is_unsigned)}

	def __str__(self):
		return f'binary_fixed({self.size})'


# endregion

# region Alias

class Alias(Statement):
	"""Aliases a new user defined type to a builtin type."""

	def __init__(self, tokens):
		super().__init__()
		self.name = _get_token_value(tokens[0])
		self.linked_type = tokens[1]

	@property
	def display_type(self):
		"""Gets the display type."""
		return self.linked_type.display_type

	@property
	def is_unsigned(self):
		"""Returns true if the underlying data is unsigned."""
		return self.linked_type.is_unsigned

	@property
	def size(self):
		"""Gets the backing size."""
		return self.linked_type.size

	def _to_legacy_descriptor(self):
		return {'name': self.name, **self.linked_type.to_legacy_descriptor()}

	def __str__(self):
		return f'using {self.name} = {self.linked_type}'


# endregion

# region Enum

class Enum(Statement):
	"""Defines an enumeration of constant values."""

	def __init__(self, tokens):
		super().__init__()
		self.name = _get_token_value(tokens[0])
		self.base = tokens[1]
		self.values = tokens[2:]

		self.display_type = DisplayType.ENUM
		self.attributes = None

	@property
	def is_unsigned(self):
		"""Returns true if the underlying data is unsigned."""
		return self.base.is_unsigned

	@property
	def is_bitwise(self):
		"""Returns true if this enumeration is composed of bit flags and should support bitwise operations."""
		return _lookup_attribute_value(self.attributes, 'is_bitwise')

	@property
	def size(self):
		"""Gets the backing size."""
		return self.base.size

	def _to_legacy_descriptor(self):
		type_descriptor = {
			'name': self.name,
			'type': 'enum',
			'size': self.base.size,
			'signedness': _format_is_unsigned(self.is_unsigned),
			'values': [value.to_legacy_descriptor() for value in self.values]
		}

		_set_if(self, type_descriptor, 'is_bitwise')
		return type_descriptor

	def __str__(self):
		return _format_attributes(self.attributes) + f'enum {self.name} : {self.base}  # {len(self.values)} value(s)'


class EnumValue(Statement):
	"""Named value within an enumeration."""

	def __init__(self, tokens):
		super().__init__()
		self.name = _get_token_value(tokens[0])
		self.value = _get_token_value(tokens[1])

	def _to_legacy_descriptor(self):
		return {'name': self.name, 'value': self.value}

	def __str__(self):
		return f'{self.name} = {self.value}'


# endregion

# region Attribute

class Attribute:
	"""Defines a user defined attribute."""

	def __init__(self, tokens):
		self.name = _get_token_value(tokens[0])

		self.is_flag = 1 == len(tokens)
		self.value = True if self.is_flag else _get_token_value(tokens[1])

		self.values = [] if self.is_flag else [_get_token_value(token) for token in tokens[1:]]

	def __str__(self):
		if self.is_flag:
			return f'@{self.name}'

		formatted_values = []
		qualifier = ''
		for value in self.values:
			if 'not' == value:
				qualifier = 'not '
			else:
				formatted_values.append(f'{qualifier}{value}')
				qualifier = ''

		return f'@{self.name}({", ".join(formatted_values)})'


# endregion

# region Struct

class Struct(Statement):  # pylint: disable=too-many-instance-attributes
	"""Defines a user defined data type."""

	Initializer = namedtuple('Initializer', ['target_property_name', 'value'])

	def __init__(self, tokens):
		super().__init__()
		self.name = _get_token_value(tokens[1])
		self.disposition = _get_token_value(tokens[0]) if tokens[0] else None
		self.fields = tokens[2:]
		self.factory_type = None

		self.display_type = DisplayType.STRUCT
		self.attributes = None

		self.requires_unaligned = False

		self._member_comment_start_regex = None

	@property
	def is_abstract(self):
		"""Returns true if this struct is abstract."""
		return 'abstract' == self.disposition

	@property
	def is_inline(self):
		"""Returns true if this struct is inline."""
		return 'inline' == self.disposition

	@property
	def is_aligned(self):
		"""Returns true if this structure is composed exclusively of aligned fields."""
		return _lookup_attribute_value(self.attributes, 'is_aligned')

	@property
	def is_size_implicit(self):
		"""Returns true if this structure can be used in a `sizeof` expression."""
		return _lookup_attribute_value(self.attributes, 'is_size_implicit')

	@property
	def size(self):
		"""Gets the name of the property containing this structure's size."""
		return _lookup_attribute_value(self.attributes, 'size')

	@property
	def discriminator(self):
		"""Gets the names of the properties that can be used to uniquely identify a concrete instantiation of this structure."""
		return _lookup_attribute_value(self.attributes, 'discriminator', True)

	@property
	def comparer(self):
		"""Gets the building blocks of a comparer."""
		values = _lookup_attribute_value(self.attributes, 'comparer', True)
		if not values:
			return values

		return [(values[2 * i], values[2 * i + 1]) for i in range(0, len(values) // 2)]

	@property
	def initializers(self):
		"""Gets field initializers, each specifying the constant with which to initialize a field."""
		if not self.attributes:
			return []

		return [
			self.Initializer(attribute.values[0], attribute.values[1])
			for attribute in self.attributes if 'initializes' == attribute.name
		]

	def apply_inline_template(self, named_inline_field):
		"""Expands a named inline field using this struct."""

		if not self.is_inline:
			raise AstException(f'apply_inline_template called for struct {self.name} not marked as inline')

		comment_map = self._build_comment_map(named_inline_field.comment) if named_inline_field.comment else {}

		# copy all fields from this inline struct
		fields_copy = [(field.name, field.copy(named_inline_field.name)) for field in self.fields]

		for (original_field_name, field) in fields_copy:
			if original_field_name in comment_map:
				field.comment = Comment('\n'.join(comment_map[original_field_name]))

		return [tuple[1] for tuple in fields_copy]

	def _to_legacy_descriptor(self):
		type_descriptor = {
			'name': self.name,
			'type': 'struct',
			'layout': [field.to_legacy_descriptor() for field in self.fields]
		}

		for property_name in ['disposition', 'factory_type', 'is_aligned', 'is_size_implicit', 'size', 'discriminator']:
			_set_if(self, type_descriptor, property_name)

		if self.comparer:
			type_descriptor['comparer'] = [
				{
					'name': property_name,
					'transform': transform
				} for (property_name, transform) in self.comparer
			]

		if self.initializers:
			type_descriptor['initializers'] = [
				{
					'target_property_name': initializer.target_property_name,
					'value': initializer.value
				} for initializer in self.initializers
			]

		return type_descriptor

	def __str__(self):
		formatted = _format_attributes(self.attributes)

		if self.disposition:
			formatted += f'{self.disposition} '

		return f'{formatted}struct {self.name}  # {len(self.fields)} field(s)'

	def _build_comment_map(self, comment):
		if not self._member_comment_start_regex:
			self._member_comment_start_regex = re.compile(r'^\[(?P<comment_key>\S+)\] ')

		comment_map = {}
		active_comment_key = None
		for line in comment.parsed.split('\n'):
			member_comment_start_match = self._member_comment_start_regex.match(line)

			if member_comment_start_match:
				active_comment_key = member_comment_start_match.group('comment_key')
				comment_map[active_comment_key] = [line[len(active_comment_key) + 3:]]
			elif active_comment_key:
				comment_map[active_comment_key].append(f'\n{line}')

		return comment_map


class StructField(Statement):
	"""Named field within a user defined structure."""

	def __init__(self, tokens, disposition=None):
		super().__init__()
		self.name = _get_token_value(tokens[0])
		self.field_type = tokens[1]  # resolve this to reference object
		self.value = _get_token_value(tokens[2]) if len(tokens) > 2 else None
		self.disposition = disposition

		self.attributes = None

	@property
	def is_const(self):
		"""Returns true if this field is const."""
		return 'const' == self.disposition

	@property
	def is_reserved(self):
		"""Returns true if this field is reserved."""
		return 'reserved' == self.disposition

	@property
	def is_size_reference(self):
		"""Returns true if this field is a size reference."""
		return 'sizeof' == self.disposition

	@property
	def is_conditional(self):
		"""Returns true if this field is conditional."""
		return isinstance(self.value, Conditional)

	@property
	def is_unsigned(self):
		"""Returns true if the underlying data is unsigned."""
		return self.field_type.is_unsigned if hasattr(self.field_type, 'is_unsigned') else None

	@property
	def display_type(self):
		"""Gets the display type."""
		return DisplayType.UNSET if isinstance(self.field_type, str) else self.field_type.display_type

	@property
	def size(self):
		"""Gets the backing size."""
		return self.field_type.size if hasattr(self.field_type, 'size') else None

	def copy(self, prefix):
		"""Creates a copy of this field and transforms field names using the specified prefix."""

		field = StructField([
			prefix if '__value__' == self.name else f'{prefix}_{self.name}',
			self.field_type.copy(prefix) if hasattr(self.field_type, 'copy') else self.field_type,
			self.value.copy(prefix) if hasattr(self.value, 'value') else self.value,
		], self.disposition)
		field.attributes = self.attributes
		return field

	def _to_legacy_descriptor(self):
		type_descriptor = {'name': self.name}
		if hasattr(self.field_type, 'to_legacy_descriptor'):
			type_descriptor.update(self.field_type.to_legacy_descriptor())
		else:
			type_descriptor['type'] = _get_token_value(self.field_type)

		if None is not self.value:
			if hasattr(self.value, 'to_legacy_descriptor'):
				type_descriptor.update(self.value.to_legacy_descriptor())
			else:
				type_descriptor['value'] = self.value

		_set_if(self, type_descriptor, 'disposition')

		return type_descriptor

	def __str__(self):
		formatted = _format_attributes(self.attributes)
		formatted += f'{self.name} = '

		if 'inline' == self.disposition:
			return formatted + f'inline {self.field_type}'

		if not self.disposition:
			return formatted + f'{self.field_type}' + ('' if not self.value else f' {str(self.value)}')

		if self.disposition in ['const', 'reserved']:
			formatted += 'make_'

		return formatted + f'{self.disposition}({self.field_type}, {self.value})'


class StructInlinePlaceholder(Statement):
	"""Placeholder within a user defined structure indicating a linked substructure."""

	def __init__(self, tokens):
		super().__init__()
		self.inlined_typename = _get_token_value(tokens[0])

	def _to_legacy_descriptor(self):
		return {'type': self.inlined_typename, 'disposition': 'inline'}

	def __str__(self):
		return f'inline {self.inlined_typename}'


class Conditional:
	"""Marks a structure field as optional and can be used to build union-like semantics."""

	def __init__(self, tokens):
		self.linked_field_name = _get_token_value(tokens[2])
		self.operation = _get_token_value(tokens[1])
		self.value = _get_token_value(tokens[0])

	def copy(self, prefix):
		"""Creates a copy of this field and transforms field names using the specified prefix."""

		return Conditional([self.value, self.operation, f'{prefix}_{self.linked_field_name}'])

	def to_legacy_descriptor(self):
		"""Produces a dictionary consistent with the original catbuffer type descriptors."""

		return {
			'condition': self.linked_field_name,
			'condition_operation': self.operation,
			'condition_value': self.value
		}

	def __str__(self):
		return f'if {self.value} {self.operation} {self.linked_field_name}'


# endregion

# region Array

class Array:
	"""Array composed of zero or more elements; can be count-based, size-based or fill."""

	def __init__(self, tokens):
		self.element_type = _get_token_value(tokens[0]) if isinstance(tokens[0], Token) else tokens[0]

		self._raw_size = _get_token_value(tokens[1])
		self.size = 0 if '__FILL__' == self._raw_size else self._raw_size

		# backing attribute fields
		self._attributes = {}

	@property
	def sort_key(self):
		"""Gets the property that should be used to sort array elements."""
		return self._attributes.get('sort_key', None)

	@sort_key.setter
	def sort_key(self, values):
		"""Sets the property that should be used to sort array elements."""
		self._attributes['sort_key'] = values[0]

	@property
	def is_byte_constrained(self):
		"""Returns true if the size value should be interpreted as a byte value instead of an element count."""
		return self._attributes.get('is_byte_constrained', False)

	@is_byte_constrained.setter
	def is_byte_constrained(self, values):
		"""Sets the interpretation of the size value."""
		self._attributes['is_byte_constrained'] = None if values is None else True

	@property
	def alignment(self):
		"""Gets the start alignment of array elements."""
		return self._attributes.get('alignment', None)

	@alignment.setter
	def alignment(self, values):
		"""Sets the alignment of array elements."""
		self._attributes['alignment'] = values[0]
		self._attributes['is_last_element_padded'] = not ('not' == values[1] and 'pad_last' == values[2])

	@property
	def is_last_element_padded(self):
		"""Returns true if the last element is padded to the alignment boundary."""
		return self._attributes.get('is_last_element_padded', None)

	@property
	def display_type(self):
		"""Gets the display type."""
		if isinstance(self.element_type, FixedSizeInteger) and 1 == self.element_type.size:
			return DisplayType.BYTE_ARRAY

		return DisplayType.TYPED_ARRAY

	@property
	def is_expandable(self):
		"""Returns true if this array expands to fill a structure."""
		return '__FILL__' == self._raw_size

	@property
	def disposition(self):
		"""Gets the disposition of this array."""
		if self.is_expandable:
			return 'array fill'

		return 'array sized' if self.is_byte_constrained else 'array'

	def copy(self, prefix):
		"""Creates a copy of this field and transforms field names using the specified prefix."""

		size = self.size if not isinstance(self.size, str) else f'{prefix}_{self.size}'
		copy = Array([self.element_type, size])
		copy._attributes = self._attributes  # pylint: disable=protected-access

		if isinstance(self.sort_key, str):
			copy.sort_key = [f'{prefix}_{self.sort_key}']

		return copy

	def to_legacy_descriptor(self):
		"""Produces a dictionary consistent with the original catbuffer type descriptors."""

		type_descriptor = {'disposition': self.disposition, 'size': self.size}

		if isinstance(self.element_type, FixedSizeInteger):
			type_descriptor.update({
				'element_disposition': {
					'size': self.element_type.size,
					'signedness': _format_is_unsigned(self.element_type.is_unsigned)
				},
				'type': 'byte'
			})
		else:
			type_descriptor['type'] = self.element_type

		for property_name in ['sort_key', 'alignment', 'is_last_element_padded']:
			_set_if(self, type_descriptor, property_name)

		return type_descriptor

	def __str__(self):
		size = '__FILL__' if 'array fill' == self.disposition else str(self.size)
		return f'array({self.element_type}, {size})'


# endregion
