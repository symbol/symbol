import unittest

from catparser.ast import Alias, AstException, FixedSizeInteger, Struct, StructField, StructInlinePlaceholder
from catparser.AstPostProcessor import AstPostProcessor


class AstPostProcessorTests(unittest.TestCase):
    @staticmethod
    def _create_type_descriptors_for_named_inline_tests():
        return [
            Alias(['AlphaAlias', FixedSizeInteger('uint16')]),
            Struct([
                None,
                'SigmaStruct',
                StructField(['counter', FixedSizeInteger('uint8')]),
                StructField(['iso', 'FooBar'], 'inline'),
                StructField(['alpha', 'AlphaAlias'])
            ]),
            Struct([
                'inline', 'FooBar', StructField(['__value__', 'MyCustomType']), StructField(['beta', FixedSizeInteger('uint16')])
            ]),
            Struct([None, 'SillyStruct', StructField(['weight', FixedSizeInteger('uint8')])])
        ]

    def test_type_descriptors_filters_out_inline_structs(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors_for_named_inline_tests())

        # Act:
        type_descriptors = processor.type_descriptors

        # Assert: FooBar was filtered out
        self.assertEqual(['AlphaAlias', 'SigmaStruct', 'SillyStruct'], [model.name for model in type_descriptors])

    def test_expand_named_inlines_fails_when_named_inline_type_is_unknown(self):
        # Arrange: create without referenced inline struct (FooBar)
        processor = AstPostProcessor(self._create_type_descriptors_for_named_inline_tests()[:2])

        # Act + Assert:
        with self.assertRaises(AstException):
            processor.expand_named_inlines()

    def test_expand_named_inlines_expands_named_inlines(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors_for_named_inline_tests())

        # Act:
        processor.expand_named_inlines()
        type_descriptors = processor.type_descriptors

        # Assert:
        self.assertEqual(['counter', 'iso', 'iso_beta', 'alpha'], [field.name for field in type_descriptors[1].fields])  # iso was expanded
        self.assertEqual(['weight'], [field.name for field in type_descriptors[2].fields])  # nothing was expanded

    @staticmethod
    def _create_type_descriptors_for_unnamed_inline_tests():
        return [
            Struct(['inline', 'Inline1', StructField(['counter', FixedSizeInteger('uint8')])]),
            Struct(['abstract', 'Abstract2', StructInlinePlaceholder(['Inline1']), StructField(['weight', FixedSizeInteger('uint32')])]),
            Struct([None, 'Plain3', StructInlinePlaceholder(['Abstract2']), StructField(['height', FixedSizeInteger('uint16')])])
        ]

    def test_type_descriptors_does_not_filter_out_abstract_structs(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors_for_unnamed_inline_tests())

        # Act:
        type_descriptors = processor.type_descriptors

        # Assert: Inline1 was filtered out
        self.assertEqual(['Abstract2', 'Plain3'], [model.name for model in type_descriptors])

    def test_expand_unnamed_inlines_fails_when_unnamed_inline_type_is_unknown(self):
        # Arrange: create without referenced inline struct (Inline1)
        processor = AstPostProcessor(self._create_type_descriptors_for_unnamed_inline_tests()[1:])

        # Act + Assert:
        with self.assertRaises(AstException):
            processor.expand_unnamed_inlines()

    def test_expand_unnamed_inlines_expands_unnamed_inlines(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors_for_unnamed_inline_tests())

        # Act:
        processor.expand_unnamed_inlines()
        type_descriptors = processor.type_descriptors

        # Assert:
        self.assertEqual(['counter', 'weight'], [field.name for field in type_descriptors[0].fields])  # Inline1 was expanded
        self.assertEqual(None, type_descriptors[0].factory_type)

        self.assertEqual(['counter', 'weight', 'height'], [field.name for field in type_descriptors[1].fields])  # Abstract2 was expanded
        self.assertEqual('Abstract2', type_descriptors[1].factory_type)
