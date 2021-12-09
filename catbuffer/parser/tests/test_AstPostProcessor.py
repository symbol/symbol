import unittest

from catparser.ast import Alias, AstException, FixedSizeInteger, Struct, StructField
from catparser.AstPostProcessor import AstPostProcessor


class AstPostProcessorTests(unittest.TestCase):
    @staticmethod
    def _create_type_descriptors():
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
            Struct([None, 'SillyStruct', StructField(['weight', FixedSizeInteger('uint8')])]),
        ]

    def test_type_descriptors_filters_out_inline_structs(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors())

        # Act:
        type_descriptors = processor.type_descriptors

        # Assert: FooBar was filtered out
        self.assertEqual(['AlphaAlias', 'SigmaStruct', 'SillyStruct'], [model.name for model in type_descriptors])

    def test_expand_named_inlines_fails_when_named_inline_type_is_unknown(self):
        # Arrange: create without referenced inline struct (FooBar)
        processor = AstPostProcessor(self._create_type_descriptors()[:2])

        # Act + Assert:
        with self.assertRaises(AstException):
            processor.expand_named_inlines()

    def test_expand_named_inlines_expands_named_inlines(self):
        # Arrange:
        processor = AstPostProcessor(self._create_type_descriptors())

        # Act:
        processor.expand_named_inlines()
        type_descriptors = processor.type_descriptors

        # Assert:
        self.assertEqual(['counter', 'iso', 'iso_beta', 'alpha'], [field.name for field in type_descriptors[1].fields])  # iso was expanded
        self.assertEqual(['weight'], [field.name for field in type_descriptors[2].fields])  # nothing was expanded
