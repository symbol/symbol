import unittest
from test.constants import (BUILTIN_TYPE_TUPLES, INVALID_PROPERTY_NAMES, INVALID_USER_TYPE_NAMES, VALID_PRIMITIVE_NAMES,
                            VALID_PROPERTY_NAMES, VALID_USER_TYPE_NAMES)
from test.ParserTestUtils import MultiLineParserTestUtils, ParserFactoryTestUtils, SingleLineParserTestUtils

from catbuffer_parser.CatsParseException import CatsParseException
from catbuffer_parser.StructParser import (StructArrayMemberParserFactory, StructConstParserFactory, StructInlineParserFactory,
                                           StructParserFactory, StructScalarMemberParserFactory)

# region StructParserTest


class StructParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(StructParserFactory, self).assert_positives([
            'struct F', 'struct Foo', 'struct FooZA09za', 'struct foo', 'struct 8oo', 'struct $^^$'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(StructParserFactory, self).assert_negatives([
            ' struct Foo', 'struct Foo ', 'struct ', 'struct foo bar'
        ])


class StructParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        MultiLineParserTestUtils(StructParserFactory, self).assert_parse(line, expected_result)

    def _assert_parse_exception(self, line):
        # Assert:
        MultiLineParserTestUtils(StructParserFactory, self).assert_parse_exception(line)

    def test_parser_exposes_custom_factories(self):
        # Act
        parser = StructParserFactory().create()

        # Assert
        self.assertEqual(4, len(parser.factories()))

    def test_can_parse_type_declaration(self):
        # Act + Assert:
        self._assert_parse(
            'struct Car',
            ('Car', {'type': 'struct', 'layout': []}))

    def test_struct_names_must_have_type_name_semantics(self):
        # Assert:
        MultiLineParserTestUtils(StructParserFactory, self).assert_naming('struct {0}', VALID_USER_TYPE_NAMES, INVALID_USER_TYPE_NAMES)

    def test_can_append_scalar(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [{'name': 'foo'}, {'name': 'bar'}]}), result)

    def test_can_append_scalar_conditional(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar', 'condition': 'foo', 'condition_value': 'red'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [
            {'name': 'foo'},
            {'name': 'bar', 'condition': 'foo', 'condition_value': 'red'}
        ]}), result)

    def test_can_append_scalar_conditional_trailing_discriminator(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'bar', 'condition': 'foo', 'condition_value': 'red'})
        parser.append({'name': 'foo'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [
            {'name': 'bar', 'condition': 'foo', 'condition_value': 'red'},
            {'name': 'foo'}
        ]}), result)

    def test_cannot_append_scalar_with_invalid_condition_reference(self):
        for condition in ['baz', '10']:
            # Arrange:
            parser = StructParserFactory().create()

            # Act:
            parser.process_line('struct Car')
            parser.append({'name': 'foo'})
            parser.append({'name': 'bar', 'condition': condition, 'condition_value': 'red'})

            # Assert:
            with self.assertRaises(CatsParseException):
                parser.commit()

    def test_can_append_array_with_numeric_size(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar', 'size': 10})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [{'name': 'foo'}, {'name': 'bar', 'size': 10}]}), result)

    def test_can_append_array_with_valid_size_reference(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar', 'size': 'foo'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [{'name': 'foo'}, {'name': 'bar', 'size': 'foo'}]}), result)

    def test_can_append_array_with_valid_size_reference_and_inline(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'disposition': 'inline', 'type': 'Vehicle'})
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar', 'size': 'foo'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [
            {'disposition': 'inline', 'type': 'Vehicle'},
            {'name': 'foo'},
            {'name': 'bar', 'size': 'foo'}
        ]}), result)

    def test_cannot_append_array_with_invalid_size_reference(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})

        # Assert:
        with self.assertRaises(CatsParseException):
            parser.append({'name': 'bar', 'size': 'fob'})

    def test_can_append_multiple_properties_with_same_name_and_different_disposition(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'foo', 'disposition': 'const'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [{'name': 'foo'}, {'name': 'foo', 'disposition': 'const'}]}), result)

    def test_cannot_append_multiple_properties_with_same_name_and_disposition(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'bar'})

        # Assert:
        with self.assertRaises(CatsParseException):
            parser.append({'name': 'foo'})

    def test_can_append_unnamed_descriptor(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'disposition': 'inline', 'type': 'Foo'})
        parser.append({'name': 'foo'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [
            {'disposition': 'inline', 'type': 'Foo'},
            {'name': 'foo'}
        ]}), result)

    def test_can_append_array_with_fill_disposition_last(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'cat', 'disposition': 'fill'})
        result = parser.commit()

        # Assert:
        self.assertEqual(('Car', {'type': 'struct', 'layout': [
            {'name': 'foo'},
            {'name': 'cat', 'disposition': 'fill'}
        ]}), result)

    def test_cannot_append_property_after_array_with_fill_disposition(self):
        # Arrange:
        parser = StructParserFactory().create()

        # Act:
        parser.process_line('struct Car')
        parser.append({'name': 'foo'})
        parser.append({'name': 'cat', 'disposition': 'fill'})

        # Assert:
        with self.assertRaises(CatsParseException):
            parser.append({'name': 'bar'})

# endregion

# region StructConstParserTest


class StructConstParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(StructConstParserFactory, self).assert_positives([
            'const Foo foo = Bar', 'const FOO FOO = BAR', 'const Za09Za FzaZa09 = fzaZa09', 'const !!! $$$ = ###'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(StructConstParserFactory, self).assert_negatives([
            ' const Foo foo = Bar', 'const Foo foo = Bar ', 'const Foo foo =', 'const Foo foo Bar', 'const Foo = Bar', 'const Foo'
        ])


class StructConstParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse(line, expected_result)

    def _assert_parse_exception(self, line):
        # Assert:
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception(line)

    def test_can_parse_uint_type_constant(self):
        # Act + Assert:
        for value in [32, 0x20]:
            self._assert_parse(
                'const uint16 foo = {0}'.format(value),
                {'name': 'foo', 'type': 'byte', 'signedness': 'unsigned', 'size': 2, 'value': 32, 'disposition': 'const'})

    def test_can_parse_custom_type_constant(self):
        # Act + Assert:
        for value in [33, 0x21]:
            self._assert_parse(
                'const ColorShade red = {0}'.format(value),
                {'name': 'red', 'type': 'ColorShade', 'value': 33, 'disposition': 'const'})

    def test_cannot_parse_non_numeric_value(self):
        # Act + Assert:
        for value in ['FOO', 'AF']:
            SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception(
                'const uint16 foo = {0}'.format(value),
                ValueError)

    def test_cannot_parse_binary_fixed_type_constant(self):
        # Act + Assert:
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception('const binary_fixed(25) foo = 123')

    def test_member_names_must_have_property_name_semantics(self):
        # Assert:
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_naming(
            'const uint32 {0} = 123',
            VALID_PROPERTY_NAMES,
            INVALID_PROPERTY_NAMES)

    def test_type_names_must_have_type_name_or_uint_semantics(self):
        # Assert:
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_naming(
            'const {0} foo = 123',
            VALID_USER_TYPE_NAMES + VALID_PRIMITIVE_NAMES,
            INVALID_USER_TYPE_NAMES + ['binary_fixed(32)'])

# endregion

# region StructInlineParserTest


class StructInlineParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(StructInlineParserFactory, self).assert_positives([
            'inline Bar', 'inline BAR', 'inline fzaZa09', 'inline $$$'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(StructInlineParserFactory, self).assert_negatives([
            ' inline Bar', 'inline Bar ', 'inline ', ' Bar'
        ])


class StructInlineParserTest(unittest.TestCase):
    def test_can_parse_simple_custom_declaration(self):
        # Act + Assert:
        SingleLineParserTestUtils(StructInlineParserFactory, self).assert_parse(
            'inline Vehicle_',
            {'type': 'Vehicle_', 'disposition': 'inline'})

# endregion

# region StructScalarMemberParser


class StructScalarParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(StructScalarMemberParserFactory, self).assert_positives([
            'foo = bar', 'foo = BAR', 'fzaZa09 = d', '& = $$$', 'foo = fazFZA90',
            'foo = bar if abc equals def', 'foo = bar if abc has def'
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(StructScalarMemberParserFactory, self).assert_negatives([
            ' foo = bar', 'foo = bar ', 'foo = ', '= bar', 'foo = array(bar, baz)', 'foo = bar if abc mask def',
            'foo = bar if abc equals', 'foo = bar abc equals def',
            'foo = bar if abc has', 'foo = bar abc has def'
        ])


class StructScalarParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        SingleLineParserTestUtils(StructScalarMemberParserFactory, self).assert_parse(line, expected_result)

    def test_can_parse_simple_custom_declaration(self):
        # Act + Assert:
        self._assert_parse(
            'car = Vehicle_',
            {'name': 'car', 'type': 'Vehicle_'})

    def test_can_parse_simple_builtin_declaration(self):
        for builtin_tuple in BUILTIN_TYPE_TUPLES:
            # Act + Assert:
            self._assert_parse(
                'car = {0}'.format(builtin_tuple[0]),
                {'name': 'car', 'type': 'byte', 'signedness': builtin_tuple[2], 'size': builtin_tuple[1]})

    def test_can_parse_conditional_custom_declaration_equals(self):
        # Act + Assert:
        self._assert_parse(
            'roadGrade = RoadGrade_ if terrain equals road',
            {'name': 'roadGrade', 'type': 'RoadGrade_', 'condition': 'terrain', 'condition_operation': 'equals', 'condition_value': 'road'})

    def test_can_parse_conditional_custom_declaration_has(self):
        # Act + Assert:
        self._assert_parse(
            'roadGrade = RoadGrade_ if terrain has road',
            {'name': 'roadGrade', 'type': 'RoadGrade_', 'condition': 'terrain', 'condition_operation': 'has', 'condition_value': 'road'})

    def test_member_names_must_have_property_name_semantics(self):
        # Assert:
        SingleLineParserTestUtils(StructScalarMemberParserFactory, self).assert_naming(
            '{0} = uint32',
            VALID_PROPERTY_NAMES,
            INVALID_PROPERTY_NAMES)

# endregion

# region StructArrayMemberParser


VALID_ARRAY_PATTERNS = ['foo = {0}(bar, {1}baz)', '$$$ = {0}(&, {1}**)', '$$$ = {0}(&, {1}**, sort_key=@@)']
INVALID_ARRAY_PATTERNS = [
    ' foo = {0}(bar, {1}baz)', 'foo = {0}(bar, {1}baz) ', 'foo = ', '= {0}(bar, {1}baz)',
    'foo = {0}(bar, {1}baz', 'foo = {0}(bar, {1}baz) if abc equals def', 'foo = {0}(bar, {1}baz) if abc has def'
]
ARRAY_DIMENSION_QUALIFIERS = ['', 'size=']


class StructArrayMemberParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        for dimension_qualifier in ARRAY_DIMENSION_QUALIFIERS:
            ParserFactoryTestUtils(StructArrayMemberParserFactory, self).assert_positives([
                pattern.format('array', dimension_qualifier) for pattern in VALID_ARRAY_PATTERNS
            ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        for dimension_qualifier in ARRAY_DIMENSION_QUALIFIERS:
            ParserFactoryTestUtils(StructArrayMemberParserFactory, self).assert_negatives([
                pattern.format('array', dimension_qualifier) for pattern in INVALID_ARRAY_PATTERNS
            ])


class StructArrayMemberParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        # Assert:
        SingleLineParserTestUtils(StructArrayMemberParserFactory, self).assert_parse(line, expected_result)

    def test_can_parse_array_with_non_numeric_size(self):
        for type_name in ['byte', 'Car']:
            # Act + Assert:
            self._assert_parse(
                'vehicles = array({0}, garageSize)'.format(type_name),
                {'name': 'vehicles', 'type': type_name, 'size': 'garageSize'})

    def test_can_parse_array_with_numeric_size(self):
        for type_name in ['byte', 'Car']:
            for numeric_str in ['10', '0x0A']:
                # Act + Assert:
                self._assert_parse(
                    'vehicles = array({0}, {1})'.format(type_name, numeric_str),
                    {'name': 'vehicles', 'type': type_name, 'size': 10})

    def test_can_parse_array_with_fill_size(self):
        for type_name in ['byte', 'Car']:
            # Act + Assert:
            self._assert_parse(
                'vehicles = array({0}, __FILL__)'.format(type_name),
                {'name': 'vehicles', 'type': type_name, 'size': 0, 'disposition': 'fill'})

    def test_can_parse_array_with_sort_key(self):
        # Act + Assert:
        self._assert_parse(
            'vehicles = array(Car, 10, sort_key=bar)',
            {'name': 'vehicles', 'type': 'Car', 'size': 10, 'sort_key': 'bar'})

    def test_can_parse_vararray_with_non_numeric_size(self):
        for type_name in ['byte', 'Car']:
            # Act + Assert:
            self._assert_parse(
                'vehicles = array({0}, size=garageSize)'.format(type_name),
                {'name': 'vehicles', 'type': type_name, 'size': 'garageSize', 'disposition': 'var'})

    def test_can_parse_vararray_with_unsupported_numeric_size(self):
        for type_name in ['byte', 'Car']:
            # Act + Assert: size is not converted for var array
            self._assert_parse(
                'vehicles = array({0}, size=0x0A)'.format(type_name),
                {'name': 'vehicles', 'type': type_name, 'size': '0x0A', 'disposition': 'var'})

    def test_can_parse_vararray_with_unsupported_fill_size(self):
        for type_name in ['byte', 'Car']:
            # Act + Assert: size is not converted for var array
            self._assert_parse(
                'vehicles = array({0}, size=__FILL__)'.format(type_name),
                {'name': 'vehicles', 'type': type_name, 'size': '__FILL__', 'disposition': 'var'})

    def test_can_parse_vararray_with_sort_key(self):
        # Act + Assert:
        self._assert_parse(
            'vehicles = array(Car, size=garageSize, sort_key=bar)',
            {'name': 'vehicles', 'type': 'Car', 'size': 'garageSize', 'disposition': 'var', 'sort_key': 'bar'})

    def test_member_names_must_have_property_name_semantics(self):
        # Assert:
        SingleLineParserTestUtils(StructArrayMemberParserFactory, self).assert_naming(
            '{0} = array(Car, 10)',
            VALID_PROPERTY_NAMES,
            INVALID_PROPERTY_NAMES)

# endregion
