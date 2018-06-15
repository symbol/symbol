# pylint: disable=invalid-name
import unittest
from parser.EnumParser import EnumParserFactory, EnumValueParserFactory
from parser.CatsParseException import CatsParseException
from test.constants import VALID_USER_TYPE_NAMES, INVALID_USER_TYPE_NAMES, UINT_TYPE_TUPLES, VALID_PROPERTY_NAMES, INVALID_PROPERTY_NAMES
from test.ParserTestUtils import MultiLineParserTestUtils, SingleLineParserTestUtils, ParserFactoryTestUtils


class EnumParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(EnumParserFactory, self).assert_positives([
            'enum F : uint8', 'enum Foo : uint7', 'enum FooZA09za : uint8', 'enum 8oo : uint9', 'enum $^^$ : uint8'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(EnumParserFactory, self).assert_negatives([
            ' enum F : uint8', 'enum F : uint8 ', 'enum F', 'enum F :', 'enum F : uintA', 'enum F : binary_fixed(8)', 'enum F : Foo'
        ])


class EnumParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        MultiLineParserTestUtils(EnumParserFactory, self).assert_parse(line, expected_result)

    def _assert_parse_exception(self, line):
        # Assert:
        MultiLineParserTestUtils(EnumParserFactory, self).assert_parse_exception(line)

    def test_parser_exposes_custom_factories(self):
        # Act
        parser = EnumParserFactory().create()

        # Assert
        self.assertEqual(1, len(parser.factories()))

    def test_can_parse_type_declaration(self):
        for uint_tuple in UINT_TYPE_TUPLES:
            # Act + Assert:
            self._assert_parse(
                'enum Colors : {0}'.format(uint_tuple[0]),
                ('Colors', {'type': 'enum', 'size': uint_tuple[1], 'values': []}))

    def test_cannot_parse_enum_declaration_with_invalid_base(self):
        for base_type in ['uint7', 'uint9']:
            # Act + Assert:
            self._assert_parse_exception('enum Colors : {0}'.format(base_type))

    def test_enum_names_must_have_type_name_semantics(self):
        # Assert:
        MultiLineParserTestUtils(EnumParserFactory, self).assert_naming(
            'enum {0} : uint8',
            VALID_USER_TYPE_NAMES,
            INVALID_USER_TYPE_NAMES)

    def test_can_append_value(self):
        # Arrange:
        parser = EnumParserFactory().create()

        # Act:
        parser.process_line('enum Colors : uint16')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Colors', {'type': 'enum', 'size': 2, 'values': [{'name': 'foo'}, {'name': 'bar'}]}), result)

    def test_cannot_append_multiple_properties_with_same_name(self):
        # Arrange:
        parser = EnumParserFactory().create()

        # Act:
        parser.process_line('enum Colors : uint16')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar'})

        # Assert:
        with self.assertRaises(CatsParseException):
            parser.append({'name': 'foo'})

    def test_can_append_multiple_properties_with_same_value(self):
        # Arrange:
        parser = EnumParserFactory().create()

        # Act:
        parser.process_line('enum Colors : uint16')
        parser.append({'name': 'foo', 'value': 2})
        parser.append({'name': 'bar', 'value': 2})
        result = parser.commit()

        # Assert:
        self.assertEqual(
            ('Colors', {'type': 'enum', 'size': 2, 'values': [{'name': 'foo', 'value': 2}, {'name': 'bar', 'value': 2}]}),
            result)


class EnumValueParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(EnumValueParserFactory, self).assert_positives([
            'foo = bar', 'foo = BAR', 'fzaZa09 = d', 'f = ccc', 'foo = fazFZA90', '$$$ = ^^^'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(EnumValueParserFactory, self).assert_negatives([
            ' foo = bar', 'foo = bar ', 'foo = ', '= bar', 'foo = array(bar, baz)'
        ])


class EnumValueParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        SingleLineParserTestUtils(EnumValueParserFactory, self).assert_parse(line, expected_result)

    def test_can_parse_dec_declaration(self):
        # Act + Assert:
        self._assert_parse(
            'red = 12',
            {'name': 'red', 'value': 12})

    def test_can_parse_hex_declaration(self):
        # Act + Assert:
        self._assert_parse(
            'red = 0x11',
            {'name': 'red', 'value': 17})

    def test_cannot_parse_non_numeric_declaration(self):
        # Act + Assert:
        SingleLineParserTestUtils(EnumValueParserFactory, self).assert_parse_exception('red = uint16', ValueError)

    def test_member_names_must_have_property_name_semantics(self):
        # Assert:
        SingleLineParserTestUtils(EnumValueParserFactory, self).assert_naming('{0} = 12', VALID_PROPERTY_NAMES, INVALID_PROPERTY_NAMES)
