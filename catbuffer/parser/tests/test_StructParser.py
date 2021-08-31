import unittest

from catparser.CatsParseException import CatsParseException
from catparser.StructParser import (StructArrayMemberParserFactory, StructConstParserFactory, StructInlineParserFactory,
                                    StructParserFactory, StructScalarMemberParserFactory)

from .constants import (BUILTIN_TYPE_TUPLES, INVALID_PROPERTY_NAMES, INVALID_USER_TYPE_NAMES, VALID_PRIMITIVE_NAMES, VALID_PROPERTY_NAMES,
                        VALID_USER_TYPE_NAMES)
from .ParserTestUtils import MultiLineParserTestUtils, ParserFactoryTestUtils, SingleLineParserTestUtils

# region StructParserTest


class StructParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        ParserFactoryTestUtils(StructParserFactory, self).assert_positives([
            'struct F', 'struct Foo', 'struct FooZA09za', 'struct foo', 'struct 8oo', 'struct $^^$'
        ])

    def test_is_match_returns_false_for_negatives(self):
        ParserFactoryTestUtils(StructParserFactory, self).assert_negatives([
            ' struct Foo', 'struct Foo ', 'struct ', 'struct foo bar'
        ])


class StructParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        MultiLineParserTestUtils(StructParserFactory, self).assert_parse(line, expected_result)

    def _assert_parse_exception(self, line):
        MultiLineParserTestUtils(StructParserFactory, self).assert_parse_exception(line)

    def test_parser_exposes_custom_factories(self):
        # Act
        parser = StructParserFactory().create()

        # Assert
        self.assertEqual(4, len(parser.factories()))

    def test_can_parse_type_declaration(self):
        self._assert_parse(
            'struct Car',
            ('Car', {'type': 'struct', 'layout': []}))

    def test_struct_names_must_have_type_name_semantics(self):
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
    @staticmethod
    def _expand_disposition_seed_patterns(seed_patterns):
        return [seed_pattern.format(disposition) for disposition in ['make_const', 'make_reserved'] for seed_pattern in seed_patterns]

    def test_is_match_returns_true_for_positives(self):
        ParserFactoryTestUtils(StructConstParserFactory, self).assert_positives(self._expand_disposition_seed_patterns([
            'foo = {}(Foo, Bar)', 'FOO = {}(FOO, BAR)', 'FzaZa09 = {}(Za09Za, fzaZa09)', '$$$ = {}(!!!, ###)'
        ]))

    def test_is_match_returns_false_for_negatives(self):
        ParserFactoryTestUtils(StructConstParserFactory, self).assert_negatives(self._expand_disposition_seed_patterns([
            ' foo = {}(Foo, Bar)', 'foo = {}(Foo, Bar) ', 'foo = {}(Foo)', 'foo = {}()',
            'foo {}(Foo, Bar)', 'foo = {}(Foo, Bar', 'foo = {} Foo, Bar'
        ]) + ['foo = make_pair(Foo, Bar)'])


class StructConstParserTest(unittest.TestCase):
    SUPPORTED_DISPOSITIONS = ['const', 'reserved']

    def _assert_parse(self, line, expected_result):
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse(line, expected_result)

    def _assert_parse_exception(self, line):
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception(line)

    def test_can_parse_uint_type_constant(self):
        for disposition in self.SUPPORTED_DISPOSITIONS:
            for value in [32, 0x20]:
                self._assert_parse(
                    'foo = make_{}(uint16, {})'.format(disposition, value),
                    {'name': 'foo', 'type': 'byte', 'signedness': 'unsigned', 'size': 2, 'value': 32, 'disposition': disposition})

    def test_can_parse_custom_type_constant(self):
        for disposition in self.SUPPORTED_DISPOSITIONS:
            for value in [33, 0x21]:
                self._assert_parse(
                    'red = make_{}(ColorShade, {})'.format(disposition, value),
                    {'name': 'red', 'type': 'ColorShade', 'value': 33, 'disposition': disposition})

    def test_cannot_parse_non_numeric_value_for_uint_type_constant(self):
        for disposition in self.SUPPORTED_DISPOSITIONS:
            for value in ['FOO', 'AF']:
                SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception(
                    'foo = make_{}(uint16, {})'.format(disposition, value),
                    ValueError)

    def test_can_parse_non_numeric_value_for_custom_type_constant(self):
        for disposition in self.SUPPORTED_DISPOSITIONS:
            for value in ['FOO', 'AF']:
                self._assert_parse(
                    'red = make_{}(ColorShade, {})'.format(disposition, value),
                    {'name': 'red', 'type': 'ColorShade', 'value': value, 'disposition': disposition})

    def test_cannot_parse_binary_fixed_type_constant(self):
        for disposition in self.SUPPORTED_DISPOSITIONS:
            SingleLineParserTestUtils(StructConstParserFactory, self).assert_parse_exception(
                'foo = make_{}(binary_fixed(25), 123)'.format(disposition))

    def test_member_names_must_have_property_name_semantics(self):
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_naming(
            '{0} = make_const(uint32, 123)',
            VALID_PROPERTY_NAMES,
            INVALID_PROPERTY_NAMES)

    def test_type_names_must_have_type_name_or_uint_semantics(self):
        SingleLineParserTestUtils(StructConstParserFactory, self).assert_naming(
            'foo = make_const({0}, 123)',
            VALID_USER_TYPE_NAMES + VALID_PRIMITIVE_NAMES,
            INVALID_USER_TYPE_NAMES + ['binary_fixed(32)'])


# endregion

# region StructInlineParserTest

class StructInlineParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        ParserFactoryTestUtils(StructInlineParserFactory, self).assert_positives([
            'inline Bar', 'inline BAR', 'inline fzaZa09', 'inline $$$'
        ])

    def test_is_match_returns_false_for_negatives(self):
        ParserFactoryTestUtils(StructInlineParserFactory, self).assert_negatives([
            ' inline Bar', 'inline Bar ', 'inline ', ' Bar'
        ])


class StructInlineParserTest(unittest.TestCase):
    def test_can_parse_simple_custom_declaration(self):
        SingleLineParserTestUtils(StructInlineParserFactory, self).assert_parse(
            'inline Vehicle_',
            {'type': 'Vehicle_', 'disposition': 'inline'})

# endregion

# region StructScalarMemberParser


class StructScalarParserFactoryTest(unittest.TestCase):
    @staticmethod
    def _expand_operation_seed_pattern(seed_pattern):
        return [seed_pattern.format(operation) for operation in ['equals', 'in', 'not equals', 'not in']]

    def test_is_match_returns_true_for_positives(self):
        ParserFactoryTestUtils(StructScalarMemberParserFactory, self).assert_positives([
            'foo = bar', 'foo = BAR', 'fzaZa09 = d', '& = $$$', 'foo = fazFZA90'
        ] + self._expand_operation_seed_pattern('foo = bar if abc {} def'))

    def test_is_match_returns_false_for_negatives(self):
        ParserFactoryTestUtils(StructScalarMemberParserFactory, self).assert_negatives([
            ' foo = bar', 'foo = bar ', 'foo = ', '= bar', 'foo = array(bar, baz)', 'foo = bar if abc mask def'
        ] + self._expand_operation_seed_pattern('foo = bar if abc {}') + self._expand_operation_seed_pattern('foo = bar abc {} def'))


class StructScalarParserTest(unittest.TestCase):
    def _assert_parse(self, line, expected_result):
        SingleLineParserTestUtils(StructScalarMemberParserFactory, self).assert_parse(line, expected_result)

    def test_can_parse_simple_custom_declaration(self):
        self._assert_parse(
            'car = Vehicle_',
            {'name': 'car', 'type': 'Vehicle_'})

    def test_can_parse_simple_builtin_declaration(self):
        for builtin_tuple in BUILTIN_TYPE_TUPLES:
            self._assert_parse(
                'car = {0}'.format(builtin_tuple[0]),
                {'name': 'car', 'type': 'byte', 'signedness': builtin_tuple[2], 'size': builtin_tuple[1]})

    def _assert_can_parse_conditional_enum_declaration(self, operation):
        self._assert_parse(
            'roadGrade = RoadGrade_ if road {} terrain'.format(operation),
            {
                'name': 'roadGrade',
                'type': 'RoadGrade_',
                'condition': 'terrain',
                'condition_operation': operation,
                'condition_value': 'road'
            })

    def test_can_parse_conditional_enum_declaration_equals(self):
        self._assert_can_parse_conditional_enum_declaration('equals')
        self._assert_can_parse_conditional_enum_declaration('not equals')

    def test_can_parse_conditional_enum_declaration_in(self):
        self._assert_can_parse_conditional_enum_declaration('in')
        self._assert_can_parse_conditional_enum_declaration('not in')

    def _assert_can_parse_conditional_byte_declaration(self, operation):
        for value in ['33', '0x21']:
            self._assert_parse(
                'roadGrade = RoadGrade_ if {} {} terrain'.format(value, operation),
                {
                    'name': 'roadGrade',
                    'type': 'RoadGrade_',
                    'condition': 'terrain',
                    'condition_operation': operation,
                    'condition_value': 33
                })

    def test_can_parse_conditional_byte_declaration_equals(self):
        self._assert_can_parse_conditional_byte_declaration('equals')
        self._assert_can_parse_conditional_byte_declaration('not equals')

    def test_can_parse_conditional_byte_declaration_in(self):
        self._assert_can_parse_conditional_byte_declaration('in')
        self._assert_can_parse_conditional_byte_declaration('not in')

    def test_member_names_must_have_property_name_semantics(self):
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
        for dimension_qualifier in ARRAY_DIMENSION_QUALIFIERS:
            ParserFactoryTestUtils(StructArrayMemberParserFactory, self).assert_positives([
                pattern.format('array', dimension_qualifier) for pattern in VALID_ARRAY_PATTERNS
            ])

    def test_is_match_returns_false_for_negatives(self):
        for dimension_qualifier in ARRAY_DIMENSION_QUALIFIERS:
            ParserFactoryTestUtils(StructArrayMemberParserFactory, self).assert_negatives([
                pattern.format('array', dimension_qualifier) for pattern in INVALID_ARRAY_PATTERNS
            ])


class StructArrayMemberParserTest(unittest.TestCase):
    DEFAULT_ARRAY_ELEMENT_TYPES = ['uint16', 'int32', 'Car']

    @staticmethod
    def _get_type_descriptor(type_name):
        if 'uint16' == type_name:
            return {'type': 'byte', 'element_disposition': {'signedness': 'unsigned', 'size': 2}}
        if 'int32' == type_name:
            return {'type': 'byte', 'element_disposition': {'signedness': 'signed', 'size': 4}}

        return {'type': type_name}

    def _assert_parse(self, line, expected_result):
        SingleLineParserTestUtils(StructArrayMemberParserFactory, self).assert_parse(line, expected_result)

    def test_can_parse_array_with_non_numeric_size(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, garageSize)'.format(type_name),
                {'name': 'vehicles', 'size': 'garageSize', 'disposition': 'array', **self._get_type_descriptor(type_name)})

    def test_can_parse_array_with_numeric_size(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            for numeric_str in ['10', '0x0A']:
                self._assert_parse(
                    'vehicles = array({0}, {1})'.format(type_name, numeric_str),
                    {'name': 'vehicles', 'size': 10, 'disposition': 'array', **self._get_type_descriptor(type_name)})

    def test_can_parse_array_with_fill_size(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, __FILL__)'.format(type_name),
                {'name': 'vehicles', 'size': 0, 'disposition': 'array fill', **self._get_type_descriptor(type_name)})

    def test_can_parse_array_with_sort_key(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, 10, sort_key=bar)'.format(type_name),
                {'name': 'vehicles', 'size': 10, 'disposition': 'array', 'sort_key': 'bar', **self._get_type_descriptor(type_name)})

    def test_can_parse_vararray_with_non_numeric_size(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, size=garageSize)'.format(type_name),
                {'name': 'vehicles', 'size': 'garageSize', 'disposition': 'array sized', **self._get_type_descriptor(type_name)})

    def test_can_parse_vararray_with_unsupported_numeric_size(self):
        # Act + Assert: size is not converted for var array
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, size=0x0A)'.format(type_name),
                {'name': 'vehicles', 'size': '0x0A', 'disposition': 'array sized', **self._get_type_descriptor(type_name)})

    def test_can_parse_vararray_with_unsupported_fill_size(self):
        # Act + Assert: size is not converted for var array
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, size=__FILL__)'.format(type_name),
                {'name': 'vehicles', 'size': '__FILL__', 'disposition': 'array sized', **self._get_type_descriptor(type_name)})

    def test_can_parse_vararray_with_sort_key(self):
        for type_name in self.DEFAULT_ARRAY_ELEMENT_TYPES:
            self._assert_parse(
                'vehicles = array({0}, size=garageSize, sort_key=bar)'.format(type_name),
                {
                    'name': 'vehicles',
                    'size': 'garageSize',
                    'disposition': 'array sized',
                    'sort_key': 'bar',
                    **self._get_type_descriptor(type_name)
                })

    def test_cannot_parse_array_with_explicit_byte_type(self):
        for size in ['100', 'size=100']:
            SingleLineParserTestUtils(StructArrayMemberParserFactory, self).assert_parse_exception(
                'vehicles = array(byte, {0})'.format(size),
                CatsParseException)

    def test_member_names_must_have_property_name_semantics(self):
        SingleLineParserTestUtils(StructArrayMemberParserFactory, self).assert_naming(
            '{0} = array(Car, 10)',
            VALID_PROPERTY_NAMES,
            INVALID_PROPERTY_NAMES)

# endregion
