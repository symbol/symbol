import unittest

from catparser.ast import (Alias, Array, ArraySeed, Comment, Conditional, Enum, EnumValue, FixedSizeBuffer, FixedSizeInteger, Struct,
                           StructField, StructInlinePlaceholder)

# region Comment


class CommentTests(unittest.TestCase):
    def test_can_parse_single_line_comment(self):
        # Act:
        model = Comment('# this is a comment')

        # Assert:
        self.assertEqual('this is a comment', model.parsed)
        self.assertEqual('this is a comment', str(model))

    def test_can_parse_multi_line_comment(self):
        # Act:
        model = Comment('\n'.join([
            '# this is a comment',
            '# foo bar'
        ]))

        # Assert:
        self.assertEqual('this is a comment foo bar', model.parsed)
        self.assertEqual('this is a comment foo bar', str(model))

    def test_can_parse_multi_line_comment_with_indentation(self):
        # Act:
        model = Comment('\n'.join([
            '# this is a comment',
            '\t# foo bar'
        ]))

        # Assert:
        self.assertEqual('this is a comment foo bar', model.parsed)
        self.assertEqual('this is a comment foo bar', str(model))

    def test_can_parse_multi_line_comment_with_newlines(self):
        # Act:
        model = Comment('\n'.join([
            '# this is a comment',
            '#',
            '# baz',
            '#',
            '# foo bar'
        ]))

        # Assert:
        self.assertEqual('this is a comment\nbaz\nfoo bar', model.parsed)
        self.assertEqual('this is a comment\nbaz\nfoo bar', str(model))

    def test_post_processing_removes_leading_and_trailing_whitespace_per_line_with_newlines(self):
        # Act:
        model = Comment('\n'.join([
            '#    this is a comment   ',
            '#            ',
            '#        baz    ',
            '#     ',
            '#      foo   bar   '
        ]))

        # Assert:
        self.assertEqual('this is a comment\nbaz\nfoo   bar', model.parsed)
        self.assertEqual('this is a comment\nbaz\nfoo   bar', str(model))


# endregion

# region FixedSizeInteger / FixedSizeBuffer

class FixedSizeIntegerTests(unittest.TestCase):
    def test_can_create_unsigned_value(self):
        # Act:
        model = FixedSizeInteger('uint16')

        # Assert:
        self.assertEqual('uint16', model.short_name)
        self.assertEqual(True, model.is_unsigned)
        self.assertEqual(2, model.size)
        self.assertEqual('unsigned', model.signedness)
        self.assertEqual({'size': 2, 'type': 'byte', 'signedness': 'unsigned'}, model.to_legacy_descriptor())
        self.assertEqual('uint16', str(model))

    def test_can_create_signed_value(self):
        # Act:
        model = FixedSizeInteger('int32')

        # Assert:
        self.assertEqual('int32', model.short_name)
        self.assertEqual(False, model.is_unsigned)
        self.assertEqual(4, model.size)
        self.assertEqual('signed', model.signedness)
        self.assertEqual({'size': 4, 'type': 'byte', 'signedness': 'signed'}, model.to_legacy_descriptor())
        self.assertEqual('int32', str(model))


class FixedSizeBufferTests(unittest.TestCase):
    def test_can_create_buffer(self):
        # Act:
        model = FixedSizeBuffer(17)

        # Assert:
        self.assertEqual(17, model.size)
        self.assertEqual({'size': 17, 'type': 'byte', 'signedness': 'unsigned'}, model.to_legacy_descriptor())
        self.assertEqual('binary_fixed(17)', str(model))


# endregion

# region Alias

class AliasTests(unittest.TestCase):
    def _test_can_create_alias(self, comment, expected_comment_descriptor):
        # Act:
        model = Alias(['FooBar', FixedSizeInteger('uint16')])
        model.comment = comment

        # Assert:
        self.assertEqual('FooBar', model.name)
        self.assertEqual('uint16', model.linked_type.short_name)
        self.assertEqual(comment, model.comment)
        self.assertEqual({
            **expected_comment_descriptor, 'name': 'FooBar', 'size': 2, 'type': 'byte', 'signedness': 'unsigned'
        }, model.to_legacy_descriptor())
        self.assertEqual('using FooBar = uint16', str(model))

    def test_can_create_alias(self):
        self._test_can_create_alias(None, {})

    def test_can_create_alias_with_comment(self):
        self._test_can_create_alias(Comment('# my amazing comment'), {'comments': 'my amazing comment'})


# endregion

# region Enum

class EnumTests(unittest.TestCase):
    def _test_can_create_enum(self, comment, expected_comment_descriptor):
        # Act:
        model = Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED', 0xFF0000]), EnumValue(['GREEN', 0x00FF00])])
        model.comment = comment

        # Assert:
        self.assertEqual('ColorShade', model.name)
        self.assertEqual('uint32', model.base.short_name)
        self.assertEqual([('RED', 0xFF0000), ('GREEN', 0x00FF00)], [(enum_value.name, enum_value.value) for enum_value in model.values])
        self.assertEqual({
            **expected_comment_descriptor,
            'name': 'ColorShade',
            'type': 'enum',
            'size': 4,
            'signedness': 'unsigned',
            'values': [{'name': 'RED', 'value': 0xFF0000}, {'name': 'GREEN', 'value': 0x00FF00}]
        }, model.to_legacy_descriptor())
        self.assertEqual('enum ColorShade : uint32  # 2 value(s)', str(model))

    def test_can_create_enum(self):
        self._test_can_create_enum(None, {})

    def test_can_create_enum_with_comment(self):
        self._test_can_create_enum(Comment('# my amazing comment'), {'comments': 'my amazing comment'})


class EnumValueTests(unittest.TestCase):
    def _test_can_create_enum_value(self, comment, expected_comment_descriptor):
        # Act:
        model = EnumValue(['AZUL', 123])
        model.comment = comment

        # Assert:
        self.assertEqual('AZUL', model.name)
        self.assertEqual(123, model.value)
        self.assertEqual(comment, model.comment)
        self.assertEqual({**expected_comment_descriptor, 'name': 'AZUL', 'value': 123}, model.to_legacy_descriptor())
        self.assertEqual('AZUL = 123', str(model))

    def test_can_create_enum_value(self):
        self._test_can_create_enum_value(None, {})

    def test_can_create_enum_value_with_comment(self):
        self._test_can_create_enum_value(Comment('# my amazing comment'), {'comments': 'my amazing comment'})


# endregion

# region Struct

class StructTests(unittest.TestCase):
    def _test_can_create_struct(self, comment, expected_comment_descriptor):
        # Act:
        model = Struct(['FooBar', StructField(['alpha', 'MyCustomType']), StructField(['beta', FixedSizeInteger('uint16')])])
        model.comment = comment

        # Assert:
        self.assertEqual('FooBar', model.name)
        self.assertEqual(['alpha', 'beta'], [field.name for field in model.fields])
        self.assertEqual({
            **expected_comment_descriptor,
            'name': 'FooBar',
            'type': 'struct',
            'layout': [{'name': 'alpha', 'type': 'MyCustomType'}, {'name': 'beta', 'size': 2, 'type': 'byte', 'signedness': 'unsigned'}]
        }, model.to_legacy_descriptor())
        self.assertEqual('struct FooBar  # 2 field(s)', str(model))

    def test_can_create_struct(self):
        self._test_can_create_struct(None, {})

    def test_can_create_struct_with_comment(self):
        self._test_can_create_struct(Comment('# my amazing comment'), {'comments': 'my amazing comment'})


class StructFieldTests(unittest.TestCase):
    def _test_can_create_struct_field(self, comment, expected_comment_descriptor):
        # Act:
        model = StructField(['foo_field', 'MyCustomType'])
        model.comment = comment

        # Assert:
        self.assertEqual('foo_field', model.name)
        self.assertEqual('MyCustomType', model.field_type)
        self.assertEqual(None, model.value)
        self.assertEqual(None, model.disposition)
        self.assertEqual(comment, model.comment)

        self.assertEqual({**expected_comment_descriptor, 'name': 'foo_field', 'type': 'MyCustomType'}, model.to_legacy_descriptor())
        self.assertEqual('foo_field = MyCustomType', str(model))

    def test_can_create_struct_field(self):
        self._test_can_create_struct_field(None, {})

    def test_can_create_struct_field_with_comment(self):
        self._test_can_create_struct_field(Comment('# my amazing comment'), {'comments': 'my amazing comment'})

    def test_can_create_conditional_struct_field(self):
        # Act:
        model = StructField(['foo_field', 'MyCustomType', Conditional(['SPECIAL_FLAG', 'not in', 'custom_flags'])])

        # Assert:
        self.assertEqual('foo_field', model.name)
        self.assertEqual('MyCustomType', model.field_type)
        self.assertEqual('custom_flags', model.value.linked_field_name)
        self.assertEqual(None, model.disposition)
        self.assertEqual(None, model.comment)

        self.assertEqual({
            'name': 'foo_field', 'type': 'MyCustomType',
            'condition': 'custom_flags', 'condition_operation': 'not in', 'condition_value': 'SPECIAL_FLAG'
        }, model.to_legacy_descriptor())
        self.assertEqual('foo_field = MyCustomType if SPECIAL_FLAG not in custom_flags', str(model))

    def _test_can_create_inline_struct_field(self, comment, expected_comment_descriptor):
        # Act:
        model = StructField(['foo_field', 'MyCustomType'], 'inline')
        model.comment = comment

        # Assert:
        self.assertEqual('foo_field', model.name)
        self.assertEqual('MyCustomType', model.field_type)
        self.assertEqual(None, model.value)
        self.assertEqual('inline', model.disposition)
        self.assertEqual(comment, model.comment)

        self.assertEqual(
            {**expected_comment_descriptor, 'name': 'foo_field', 'type': 'MyCustomType', 'disposition': 'inline'},
            model.to_legacy_descriptor())
        self.assertEqual('foo_field = inline MyCustomType', str(model))

    def test_can_create_inline_struct_field(self):
        self._test_can_create_inline_struct_field(None, {})

    def test_can_create_inline_struct_field_with_comment(self):
        self._test_can_create_inline_struct_field(Comment('# my amazing comment'), {'comments': 'my amazing comment'})

    def _test_can_create_const_struct_field(self, comment, expected_comment_descriptor):
        # Act:
        model = StructField(['BACKGROUND_COLOR', 'ColorShade', 'RED'], 'const')
        model.comment = comment

        # Assert:
        self.assertEqual('BACKGROUND_COLOR', model.name)
        self.assertEqual('ColorShade', model.field_type)
        self.assertEqual('RED', model.value)
        self.assertEqual('const', model.disposition)
        self.assertEqual(comment, model.comment)

        self.assertEqual(
            {**expected_comment_descriptor, 'name': 'BACKGROUND_COLOR', 'type': 'ColorShade', 'value': 'RED', 'disposition': 'const'},
            model.to_legacy_descriptor())
        self.assertEqual('BACKGROUND_COLOR = make_const(ColorShade, RED)', str(model))

    def test_can_create_const_struct_field(self):
        self._test_can_create_const_struct_field(None, {})

    def test_can_create_const_struct_field_with_comment(self):
        self._test_can_create_const_struct_field(Comment('# my amazing comment'), {'comments': 'my amazing comment'})

    def _test_can_create_reserved_struct_field(self, comment, expected_comment_descriptor):
        # Act:
        model = StructField(['background_color', 'ColorShade', 'RED'], 'reserved')
        model.comment = comment

        # Assert:
        self.assertEqual('background_color', model.name)
        self.assertEqual('ColorShade', model.field_type)
        self.assertEqual('RED', model.value)
        self.assertEqual('reserved', model.disposition)
        self.assertEqual(comment, model.comment)

        self.assertEqual(
            {**expected_comment_descriptor, 'name': 'background_color', 'type': 'ColorShade', 'value': 'RED', 'disposition': 'reserved'},
            model.to_legacy_descriptor())
        self.assertEqual('background_color = make_reserved(ColorShade, RED)', str(model))

    def test_can_create_reserved_struct_field(self):
        self._test_can_create_reserved_struct_field(None, {})

    def test_can_create_reserved_struct_field_with_comment(self):
        self._test_can_create_reserved_struct_field(Comment('# my amazing comment'), {'comments': 'my amazing comment'})

    def _test_can_create_const_reserved_struct_field_with_zero_value(self, field_name, disposition):
        # Act:
        model = StructField([field_name, FixedSizeInteger('uint8'), 0], disposition)

        # Assert:
        self.assertEqual(field_name, model.name)
        self.assertEqual('uint8', model.field_type.short_name)
        self.assertEqual(0, model.value)
        self.assertEqual(disposition, model.disposition)
        self.assertEqual(None, model.comment)

        self.assertEqual(
            {'name': field_name, 'type': 'byte', 'size': 1, 'signedness': 'unsigned', 'value': 0, 'disposition': disposition},
            model.to_legacy_descriptor())
        self.assertEqual(f'{field_name} = make_{disposition}(uint8, 0)', str(model))

    def test_can_create_const_struct_field_with_zero_value(self):
        self._test_can_create_const_reserved_struct_field_with_zero_value('CONST_1', 'const')

    def test_can_create_reserved_struct_field_with_zero_value(self):
        self._test_can_create_const_reserved_struct_field_with_zero_value('reserved_1', 'reserved')


class StructInlinePlaceholderTests(unittest.TestCase):
    def _test_can_create_struct_inline_placeholder(self, comment, expected_comment_descriptor):
        # Act:
        model = StructInlinePlaceholder(['MyCustomType'])
        model.comment = comment

        # Assert:
        self.assertEqual('MyCustomType', model.inlined_typename)
        self.assertEqual({**expected_comment_descriptor, 'type': 'MyCustomType', 'disposition': 'inline'}, model.to_legacy_descriptor())
        self.assertEqual('inline MyCustomType', str(model))

    def test_can_create_struct_inline_placeholder(self):
        self._test_can_create_struct_inline_placeholder(None, {})

    def test_can_create_struct_inline_placeholder_with_comment(self):
        self._test_can_create_struct_inline_placeholder(Comment('# my amazing comment'), {'comments': 'my amazing comment'})


class ConditionalTests(unittest.TestCase):
    def test_can_create_conditional(self):
        # Act:
        model = Conditional(['SPECIAL_FLAG', 'not in', 'custom_flags'])

        # Assert:
        self.assertEqual('custom_flags', model.linked_field_name)
        self.assertEqual('not in', model.operation)
        self.assertEqual('SPECIAL_FLAG', model.value)
        self.assertEqual(
            {'condition': 'custom_flags', 'condition_operation': 'not in', 'condition_value': 'SPECIAL_FLAG'},
            model.to_legacy_descriptor())
        self.assertEqual('if SPECIAL_FLAG not in custom_flags', str(model))


# endregion

# region Array

class ArraySeedTests(unittest.TestCase):
    def test_can_create_seed_array(self):
        # Act:
        seed = ArraySeed([10, 'foo_field'], 'array')

        # Assert:
        self.assertEqual(10, seed.size)
        self.assertEqual('foo_field', seed.sort_key)
        self.assertEqual('array', seed.disposition)

    def test_can_create_seed_array_sized(self):
        # Act:
        seed = ArraySeed([10], 'array sized')

        # Assert:
        self.assertEqual(10, seed.size)
        self.assertEqual(None, seed.sort_key)
        self.assertEqual('array sized', seed.disposition)

    def test_can_create_seed_array_fill(self):
        # Act:
        seed = ArraySeed([], 'array fill')

        # Assert:
        self.assertEqual(0, seed.size)
        self.assertEqual(None, seed.sort_key)
        self.assertEqual('array fill', seed.disposition)


class ArrayTests(unittest.TestCase):
    def test_can_create_udt_array(self):
        # Act:
        model = Array(['ElementType', ArraySeed([10], 'array')])

        # Assert:
        self.assertEqual(10, model.size)
        self.assertEqual('array', model.disposition)
        self.assertEqual('ElementType', model.element_type)
        self.assertEqual(None, model.sort_key)
        self.assertEqual({'disposition': 'array', 'size': 10, 'type': 'ElementType'}, model.to_legacy_descriptor())
        self.assertEqual('array(ElementType, 10)', str(model))

    def test_can_create_udt_array_with_sort_key(self):
        # Act:
        model = Array(['ElementType', ArraySeed([10, 'foo_field'], 'array')])

        # Assert:
        self.assertEqual(10, model.size)
        self.assertEqual('array', model.disposition)
        self.assertEqual('ElementType', model.element_type)
        self.assertEqual('foo_field', model.sort_key)
        self.assertEqual({'disposition': 'array', 'size': 10, 'type': 'ElementType', 'sort_key': 'foo_field'}, model.to_legacy_descriptor())
        self.assertEqual('array(ElementType, 10, sort_key=foo_field)', str(model))

    def test_can_create_int_array(self):
        # Act:
        model = Array([FixedSizeInteger('int32'), ArraySeed([12], 'array')])

        # Assert:
        self.assertEqual(12, model.size)
        self.assertEqual('array', model.disposition)
        self.assertEqual('int32', model.element_type.short_name)
        self.assertEqual(None, model.sort_key)
        self.assertEqual(
            {'disposition': 'array', 'size': 12, 'type': 'byte', 'element_disposition': {'size': 4, 'signedness': 'signed'}},
            model.to_legacy_descriptor())
        self.assertEqual('array(int32, 12)', str(model))


# endregion
