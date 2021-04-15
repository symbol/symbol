import unittest
from test.constants import (BUILTIN_TYPE_TUPLES, INT_TYPE_TUPLES, INVALID_PRIMITIVE_NAMES, INVALID_PROPERTY_NAMES, INVALID_USER_TYPE_NAMES,
                            UINT_TYPE_TUPLES, VALID_PRIMITIVE_NAMES, VALID_PROPERTY_NAMES, VALID_USER_TYPE_NAMES)

from catbuffer_parser.CatsParseException import CatsParseException
from catbuffer_parser.parserutils import (is_builtin, is_dec_or_hex, is_primitive, parse_builtin, parse_dec_or_hex, require_primitive,
                                          require_property_name, require_user_type_name)

# region naming conventions


class RequireUserTypeNameTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for string in VALID_USER_TYPE_NAMES:
            # Act:
            result = require_user_type_name(string)

            # Assert:
            self.assertEqual(string, result)

    def test_throw_for_negatives(self):
        for string in INVALID_USER_TYPE_NAMES:
            # Act:
            with self.assertRaises(CatsParseException):
                require_user_type_name(string)


class RequirePropertyNameTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for string in VALID_PROPERTY_NAMES:
            # Act:
            result = require_property_name(string)

            # Assert:
            self.assertEqual(string, result)

    def test_throw_for_negatives(self):
        for string in INVALID_PROPERTY_NAMES:
            # Act:
            with self.assertRaises(CatsParseException):
                require_property_name(string)

# endregion

# region primitive


class IsPrimitiveTest(unittest.TestCase):
    def test_true_for_positives(self):
        for string in VALID_PRIMITIVE_NAMES:
            # Act:
            result = is_primitive(string)

            # Assert:
            self.assertTrue(result)

    def test_false_for_negatives(self):
        for string in INVALID_PRIMITIVE_NAMES:
            # Act:
            result = is_primitive(string)

            # Assert:
            self.assertFalse(result)


class RequirePrimitiveTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for string in VALID_PRIMITIVE_NAMES:
            # Act:
            result = require_primitive(string)

            # Assert:
            self.assertEqual(string, result)

    def test_throw_for_negatives(self):
        for string in INVALID_PRIMITIVE_NAMES:
            # Act:
            with self.assertRaises(CatsParseException):
                require_primitive(string)

# endregion

# region dec or hex


INVALID_NUMERIC_STRINGS = ['AFE', '0x8Y8', 'p', '&']


class IsDecOrHexTest(unittest.TestCase):
    def test_true_for_positives(self):
        for string in ['10', '123', '0x10', '0x123', '0xAFE']:
            # Act:
            result = is_dec_or_hex(string)

            # Assert:
            self.assertTrue(result)

    def test_false_for_negatives(self):
        for string in INVALID_NUMERIC_STRINGS:
            # Act:
            result = is_dec_or_hex(string)

            # Assert:
            self.assertFalse(result)


class ParseDecOrHexTest(unittest.TestCase):
    def test_can_parse_dec(self):
        # Act + Assert:
        self.assertEqual(10, parse_dec_or_hex('10'))
        self.assertEqual(123, parse_dec_or_hex('123'))

    def test_can_parse_hex(self):
        # Act + Assert:
        self.assertEqual(0x10, parse_dec_or_hex('0x10'))
        self.assertEqual(0x123, parse_dec_or_hex('0x123'))
        self.assertEqual(0xAFE, parse_dec_or_hex('0xAFE'))

    def test_cannot_parse_invalid_number(self):
        for string in INVALID_NUMERIC_STRINGS:
            with self.assertRaises(ValueError):
                parse_dec_or_hex(string)

# endregion

# region builtin


INVALID_BUILTIN_TYPE_NAMES = [
    'binary_fixed(2x22)',  # malformed number
    'binary_fixed(0x8Y8)',
    'binary_fixed(x)',
    'uint33',  # unsupported size
    ' uint32',  # invalid spacing
    'uint32 ',
    'FooBar'  # non-builtin
]


class IsBuiltinTest(unittest.TestCase):
    def test_true_for_positives(self):
        for builtin_tuple in BUILTIN_TYPE_TUPLES:
            # Act:
            result = is_builtin(builtin_tuple[0])

            # Assert:
            self.assertTrue(result)

    def test_false_for_negatives(self):
        for string in INVALID_BUILTIN_TYPE_NAMES:
            # Act:
            result = is_builtin(string)

            # Assert:
            self.assertFalse(result)


class ParseBuiltinTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Act:
        result = parse_builtin(line)

        # Assert:
        self.assertEqual(expected_result, result)

    def test_can_parse_int_builtin(self):
        for int_tuple in INT_TYPE_TUPLES:
            # Act + Assert:
            self._assert_parse(
                int_tuple[0],
                {'type': 'byte', 'signedness': 'signed', 'size': int_tuple[1]})

    def test_can_parse_uint_builtin(self):
        for uint_tuple in UINT_TYPE_TUPLES:
            # Act + Assert:
            self._assert_parse(
                uint_tuple[0],
                {'type': 'byte', 'signedness': 'unsigned', 'size': uint_tuple[1]})

    def test_can_parse_binary_fixed_builtin(self):
        for size_tuple in [('32', 32), ('0x20', 32), ('25', 25)]:
            # Act + Assert:
            self._assert_parse(
                'binary_fixed({0})'.format(size_tuple[0]),
                {'type': 'byte', 'signedness': 'unsigned', 'size': size_tuple[1]})

    def test_cannot_parse_invalid_builtin(self):
        # Arrange:
        for type_name in INVALID_BUILTIN_TYPE_NAMES:
            # Act + Assert:
            with self.assertRaises(CatsParseException):
                parse_builtin(type_name)

# endregion
