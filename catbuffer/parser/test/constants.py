VALID_USER_TYPE_NAMES = ['FooBar', 'Foo', 'FooBarBaz', 'Foo123', 'Foo123BAZ']
INVALID_USER_TYPE_NAMES = ['fooBar', 'foo_bar', 'Foo_Bar', '_fooBar', 'm_fooBar', 'FOO_BAR']

VALID_PROPERTY_NAMES = ['fooBar', 'foo', 'fooBarBaz', 'foo123', 'foo123BAZ']
INVALID_PROPERTY_NAMES = ['FooBar', 'Foo_bar', 'foo_Bar', '_fooBar', 'm_fooBar', 'FOO_BAR']

VALID_UINT_NAMES = ['uint8', 'uint16', 'uint32', 'uint64']
INVALID_UINT_NAMES = [' uint8', 'uint16 ', 'uint 32', 'uint63', 'foo']

UINT_TYPE_TUPLES = [('uint8', 1), ('uint16', 2), ('uint32', 4), ('uint64', 8)]
BUILTIN_TYPE_TUPLES = UINT_TYPE_TUPLES + [('binary_fixed(32)', 32), ('binary_fixed(0x20)', 32), ('binary_fixed(25)', 25)]
