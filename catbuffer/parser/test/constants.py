VALID_USER_TYPE_NAMES = ['FooBar', 'Foo', 'FooBarBaz', 'Foo123', 'Foo123BAZ']
INVALID_USER_TYPE_NAMES = ['fooBar', 'foo_bar', 'Foo_Bar', '_fooBar', 'm_fooBar', 'FOO_BAR']

VALID_PROPERTY_NAMES = ['fooBar', 'foo', 'fooBarBaz', 'foo123', 'foo123BAZ']
INVALID_PROPERTY_NAMES = ['FooBar', 'Foo_bar', 'foo_Bar', '_fooBar', 'm_fooBar', 'FOO_BAR']

VALID_PRIMITIVE_NAMES = ['int8', 'int16', 'int32', 'int64', 'uint8', 'uint16', 'uint32', 'uint64']
INVALID_PRIMITIVE_NAMES = ['sint8', 'vint32', ' uint8', 'uint16 ', 'uint 32', 'uint63', 'foo']

INT_TYPE_TUPLES = [('int8', 1, 'signed'), ('int16', 2, 'signed'), ('int32', 4, 'signed'), ('int64', 8, 'signed')]
UINT_TYPE_TUPLES = [('uint8', 1, 'unsigned'), ('uint16', 2, 'unsigned'), ('uint32', 4, 'unsigned'), ('uint64', 8, 'unsigned')]
PRIMITIVE_TYPE_TUPLES = INT_TYPE_TUPLES + UINT_TYPE_TUPLES
BUILTIN_TYPE_TUPLES = PRIMITIVE_TYPE_TUPLES + [
    ('binary_fixed(32)', 32, 'unsigned'), ('binary_fixed(0x20)', 32, 'unsigned'), ('binary_fixed(25)', 25, 'unsigned')
]
