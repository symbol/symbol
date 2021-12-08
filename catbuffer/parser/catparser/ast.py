from abc import ABC, abstractmethod

from lark import Token


def _get_token_value(token):
    return token.value if isinstance(token, Token) else token


# region Statement

class Statement(ABC):
    """Base class that allows comments to be attached to a top-level or sub-level declaration."""

    def __init__(self):
        self.comment = None

    @abstractmethod
    def _to_legacy_descriptor(self):
        pass

    def to_legacy_descriptor(self):
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
            comment_line = comment_line.strip()[1:].strip()  # strip '#' and whitespace

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
        return {'size': self.size, 'type': 'byte', 'signedness': self.signedness}

    def __str__(self):
        return self.short_name


class FixedSizeBuffer:
    """Fixed size buffer composed of a constant number of bytes."""

    def __init__(self, size):
        self.size = size

    def to_legacy_descriptor(self):
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

# Struct

class Struct(Statement):
    """Defines a user defined data type."""

    def __init__(self, tokens):
        super().__init__()
        self.name = _get_token_value(tokens[0])
        self.fields = tokens[1:]

    def _to_legacy_descriptor(self):
        return {
            'name': self.name,
            'type': 'struct',
            'layout': [field.to_legacy_descriptor() for field in self.fields]
        }

    def __str__(self):
        return f'struct {self.name}  # {len(self.fields)} field(s)'


class StructField(Statement):
    """Named field within a user defined structure."""

    def __init__(self, tokens, disposition=None):
        super().__init__()
        self.name = _get_token_value(tokens[0])
        self.field_type = tokens[1]  # resolve this to reference object
        self.value = _get_token_value(tokens[2]) if len(tokens) > 2 else None
        self.disposition = disposition

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

        if self.disposition:
            type_descriptor['disposition'] = self.disposition

        return type_descriptor

    def __str__(self):
        if 'inline' == self.disposition:
            return f'{self.name} = inline {self.field_type}'
        if self.disposition in ['const', 'reserved']:
            return f'{self.name} = make_{self.disposition}({self.field_type}, {self.value})'

        return f'{self.name} = {self.field_type}' + ('' if not self.value else f' {str(self.value)}')


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

    def to_legacy_descriptor(self):
        return {
            'condition': self.linked_field_name,
            'condition_operation': self.operation,
            'condition_value': self.value
        }

    def __str__(self):
        return f'if {self.value} {self.operation} {self.linked_field_name}'


# endregion

# region Array

class ArraySeed:
    """Used to initialize an Array."""

    def __init__(self, tokens, disposition):
        self.size = _get_token_value(tokens[0]) if len(tokens) > 0 else 0
        self.sort_key = _get_token_value(tokens[1]) if len(tokens) > 1 else None
        self.disposition = disposition


class Array:
    """Array composed of zero or more elements; can be count-based, size-based or fill."""

    def __init__(self, tokens):
        # tokens[1] is ArraySeed
        self.size = tokens[1].size
        self.disposition = tokens[1].disposition
        self.sort_key = tokens[1].sort_key
        self.element_type = _get_token_value(tokens[0]) if isinstance(tokens[0], Token) else tokens[0]  # resolve this to reference object

    def to_legacy_descriptor(self):
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

        if self.sort_key:
            type_descriptor['sort_key'] = self.sort_key

        return type_descriptor

    def __str__(self):
        if self.sort_key:
            return f'array({self.element_type}, {self.size}, sort_key={self.sort_key})'

        return f'array({self.element_type}, {self.size})'

# endregion
