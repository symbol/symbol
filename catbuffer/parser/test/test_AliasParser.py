import unittest
from test.constants import BUILTIN_TYPE_TUPLES, INVALID_USER_TYPE_NAMES, VALID_USER_TYPE_NAMES
from test.ParserTestUtils import ParserFactoryTestUtils, SingleLineParserTestUtils

from catbuffer_parser.AliasParser import AliasParserFactory


class AliasParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(AliasParserFactory, self).assert_positives([
            'using A = foo', 'using ^ = $$$', 'using A90zaZa = te$t'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(AliasParserFactory, self).assert_negatives([
            ' using A = foo', 'using A = foo ', 'import A = foo', 'using A = foo bar', 'using A B = foo bar'
        ])


class AliasParserTest(unittest.TestCase):
    def test_can_parse_builtin_as_alias(self):
        for builtin_tuple in BUILTIN_TYPE_TUPLES:
            # Act + Assert:
            SingleLineParserTestUtils(AliasParserFactory, self).assert_parse(
                'using Age = {0}'.format(builtin_tuple[0]),
                ('Age', {'type': 'byte', 'signedness': builtin_tuple[2], 'size': builtin_tuple[1]}))

    def test_alias_names_must_have_type_name_semantics(self):
        # Assert:
        SingleLineParserTestUtils(AliasParserFactory, self).assert_naming(
            'using {0} = uint32',
            VALID_USER_TYPE_NAMES,
            INVALID_USER_TYPE_NAMES)

    def test_cannot_parse_invalid_alias(self):
        # Arrange:
        SingleLineParserTestUtils(AliasParserFactory, self).assert_parse_exceptions([
            'using Hash256 = binary_fixed(2x22)',  # malformed number
            'using Hash256 = binary_fixed(x)',  # malformed number
            'using Age = uint33',  # unknown type
            'using Age = FooBar'  # user type
        ])
