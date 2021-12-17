import re
from abc import ABC, abstractmethod
from collections import namedtuple

from lark import Token


class AstException(Exception):
    """Exception raised when an AST violation is detected"""


def _get_token_value(token):
    return token.value if isinstance(token, Token) else token


def _set_if(source, type_descriptor, property_name):
    value = getattr(source, property_name)
    if value:
        type_descriptor[property_name] = value


def _format_attributes(attributes):
    if not attributes:
        return ''

    return '\n'.join(str(attribute) for attribute in attributes) + '\n'


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

# region FixedSizeInteger / FixedSizeBuffer

class FixedSizeInteger:
    """Signed or unsigned integer type composed of 1, 2, 4 or 8 bytes."""

    def __init__(self, string):
        self.short_name = string

        self.is_unsigned = 'u' == string[0]
        self.size = int(string[3 + (1 if self.is_unsigned else 0):]) // 8

    @property
    def signedness(self):
        return 'unsigned' if self.is_unsigned else 'signed'

    def to_legacy_descriptor(self):
        """Produces a dictionary consistent with the original catbuffer type descriptors."""

        return {'size': self.size, 'type': 'byte', 'signedness': self.signedness}

    def __str__(self):
        return self.short_name


class FixedSizeBuffer:
    """Fixed size buffer composed of a constant number of bytes."""

    def __init__(self, size):
        self.size = size

    def to_legacy_descriptor(self):
        """Produces a dictionary consistent with the original catbuffer type descriptors."""

        return {'size': self.size, 'type': 'byte', 'signedness': 'unsigned'}

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

    def _to_legacy_descriptor(self):
        return {
            'name': self.name,
            'type': 'enum',
            'size': self.base.size,
            'signedness': self.base.signedness,
            'values': [value.to_legacy_descriptor() for value in self.values]
        }

    def __str__(self):
        return f'enum {self.name} : {self.base}  # {len(self.values)} value(s)'


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

        return f'@{self.name}({", ".join(self.values)})'


# endregion

# region Struct

class Struct(Statement):
    """Defines a user defined data type."""

    Initializer = namedtuple('Initializer', ['target_property_name', 'value'])

    def __init__(self, tokens):
        super().__init__()
        self.name = _get_token_value(tokens[1])
        self.disposition = _get_token_value(tokens[0]) if tokens[0] else None
        self.fields = tokens[2:]
        self.factory_type = None

        self.attributes = None

        self._member_comment_start_regex = None

    @property
    def is_inline(self):
        return 'inline' == self.disposition

    @property
    def size(self):
        return self._lookup_attribute_value('size')

    @property
    def discriminator(self):
        return self._lookup_attribute_value('discriminator')

    @property
    def initializers(self):
        if not self.attributes:
            return []

        return [
            self.Initializer(attribute.values[0], attribute.values[1])
            for attribute in self.attributes if 'initializes' == attribute.name
        ]

    def _lookup_attribute_value(self, name):
        if not self.attributes:
            return None

        attribute = next((attribute for attribute in self.attributes if attribute.name == name), None)
        return None if not attribute else attribute.value

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

        for property_name in ['disposition', 'factory_type', 'size', 'discriminator']:
            _set_if(self, type_descriptor, property_name)

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
        if self.disposition in ['const', 'reserved']:
            return formatted + f'make_{self.disposition}({self.field_type}, {self.value})'

        return formatted + f'{self.field_type}' + ('' if not self.value else f' {str(self.value)}')


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

        # attributes
        self.sort_key = None
        self.alignment = None
        self.is_byte_constrained = False

    @property
    def disposition(self):
        if '__FILL__' == self._raw_size:
            return 'array fill'

        return 'array sized' if self.is_byte_constrained else 'array'

    def copy(self, prefix):
        """Creates a copy of this field and transforms field names using the specified prefix."""

        size = self.size if not isinstance(self.size, str) else f'{prefix}_{self.size}'
        copy = Array([self.element_type, size])
        copy.sort_key = self.sort_key if not isinstance(self.sort_key, str) else f'{prefix}_{self.sort_key}'
        copy.alignment = self.alignment
        copy.is_byte_constrained = self.is_byte_constrained
        return copy

    def to_legacy_descriptor(self):
        """Produces a dictionary consistent with the original catbuffer type descriptors."""

        type_descriptor = {'disposition': self.disposition, 'size': self.size}

        if isinstance(self.element_type, FixedSizeInteger):
            type_descriptor.update({
                'element_disposition': {
                    'size': self.element_type.size,
                    'signedness': self.element_type.signedness
                },
                'type': 'byte'
            })
        else:
            type_descriptor['type'] = self.element_type

        for property_name in ['sort_key', 'alignment']:
            _set_if(self, type_descriptor, property_name)

        return type_descriptor

    def __str__(self):
        size = '__FILL__' if 'array fill' == self.disposition else str(self.size)
        return f'array({self.element_type}, {size})'


# endregion
