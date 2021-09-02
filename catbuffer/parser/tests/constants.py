INT_TYPE_TUPLES = [('int8', 1, 'signed'), ('int16', 2, 'signed'), ('int32', 4, 'signed'), ('int64', 8, 'signed')]
UINT_TYPE_TUPLES = [('uint8', 1, 'unsigned'), ('uint16', 2, 'unsigned'), ('uint32', 4, 'unsigned'), ('uint64', 8, 'unsigned')]
PRIMITIVE_TYPE_TUPLES = INT_TYPE_TUPLES + UINT_TYPE_TUPLES
BUILTIN_TYPE_TUPLES = PRIMITIVE_TYPE_TUPLES + [
    ('binary_fixed(32)', 32, 'unsigned'), ('binary_fixed(0x20)', 32, 'unsigned'), ('binary_fixed(25)', 25, 'unsigned')
]


class NameConstants:
    VALID_USER_TYPES = ['FooBar', 'Foo', 'FooBarBaz', 'Foo123', 'Foo123BAZ', 'Foo9']
    VALID_PROPERTIES = ['foo_bar', 'foo', 'foo_bar_baz', 'foo_123', 'foo_123_baz', 'm_foo_bar', '_foo_bar', '__foo__', 'foo9']
    VALID_CONST_PROPERTIES = ['FOO_BAR', 'FOO', 'FOO_BAR_BAZ', 'FOO_123', 'FOO_123_BAZ', 'M_FOO_BAR', '_FOO_BAR', '__FOO__', 'FOO9']
    VALID_PRIMITIVES = ['int8', 'int16', 'int32', 'int64', 'uint8', 'uint16', 'uint32', 'uint64']

    INVALID_ALL = ['fooBar', 'foo_Bar', 'F_bar', 'm_fooBar', '_fooBar', 'Foo_bar']

    INVALID_USER_TYPES = VALID_PROPERTIES + VALID_CONST_PROPERTIES + VALID_PRIMITIVES + INVALID_ALL + ['1Foo']
    INVALID_PROPERTIES = VALID_USER_TYPES + VALID_CONST_PROPERTIES + VALID_PRIMITIVES + INVALID_ALL + ['1foo']
    INVALID_CONST_PROPERTIES = VALID_USER_TYPES + VALID_PROPERTIES + VALID_PRIMITIVES + INVALID_ALL + ['1FOO']

    INVALID_PRIMITIVES = VALID_USER_TYPES + VALID_PROPERTIES + VALID_CONST_PROPERTIES + [
        'sint8', 'vint32', ' uint8', 'uint16 ', 'uint 32', 'uint63', 'foo'
    ]
