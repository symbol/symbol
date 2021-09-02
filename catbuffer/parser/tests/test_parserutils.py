import unittest

from catparser.CatsParseException import CatsParseException
from catparser.parserutils import TypeNameChecker, is_builtin, is_dec_or_hex, is_primitive, parse_builtin, parse_dec_or_hex

from .constants import BUILTIN_TYPE_TUPLES, INT_TYPE_TUPLES, UINT_TYPE_TUPLES, NameConstants

# region naming conventions


class RequireUserTypeTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for name in NameConstants.VALID_USER_TYPES:
            # Act:
            result = TypeNameChecker.require_user_type(name)

            # Assert:
            self.assertEqual(name, result)

    def test_throw_for_negatives(self):
        for name in NameConstants.INVALID_USER_TYPES:
            with self.assertRaises(CatsParseException):
                TypeNameChecker.require_user_type(name)


class RequireConstPropertyTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for name in NameConstants.VALID_CONST_PROPERTIES:
            # Act:
            result = TypeNameChecker.require_const_property(name)

            # Assert:
            self.assertEqual(name, result)

    def test_throw_for_negatives(self):
        for name in NameConstants.INVALID_CONST_PROPERTIES:
            with self.assertRaises(CatsParseException):
                TypeNameChecker.require_const_property(name)


class RequirePropertyTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for name in NameConstants.VALID_PROPERTIES:
            # Act:
            result = TypeNameChecker.require_property(name)

            # Assert:
            self.assertEqual(name, result)

    def test_throw_for_negatives(self):
        for name in NameConstants.INVALID_PROPERTIES:
            with self.assertRaises(CatsParseException):
                TypeNameChecker.require_property(name)


class RequirePrimitiveTest(unittest.TestCase):
    def test_nothrow_for_positives(self):
        for name in NameConstants.VALID_PRIMITIVES:
            # Act:
            result = TypeNameChecker.require_primitive(name)

            # Assert:
            self.assertEqual(name, result)

    def test_throw_for_negatives(self):
        for name in NameConstants.INVALID_PRIMITIVES:
            with self.assertRaises(CatsParseException):
                TypeNameChecker.require_primitive(name)


# endregion

# region primitive

class IsPrimitiveTest(unittest.TestCase):
    def test_true_for_positives(self):
        for name in NameConstants.VALID_PRIMITIVES:
            # Act:
            result = is_primitive(name)

            # Assert:
            self.assertTrue(result)

    def test_false_for_negatives(self):
        for name in NameConstants.INVALID_PRIMITIVES:
            # Act:
            result = is_primitive(name)

            # Assert:
            self.assertFalse(result)


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
        self.assertEqual(10, parse_dec_or_hex('10'))
        self.assertEqual(123, parse_dec_or_hex('123'))

    def test_can_parse_hex(self):
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
        for name in INVALID_BUILTIN_TYPE_NAMES:
            # Act:
            result = is_builtin(name)

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
                'binary_fixed({})'.format(size_tuple[0]),
                {'type': 'byte', 'signedness': 'unsigned', 'size': size_tuple[1]})

    def test_cannot_parse_invalid_builtin(self):
        # Arrange:
        for type_name in INVALID_BUILTIN_TYPE_NAMES:
            # Act + Assert:
            with self.assertRaises(CatsParseException):
                parse_builtin(type_name)

# endregion
