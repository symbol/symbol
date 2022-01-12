import unittest

from catparser.ast import Alias, Array, AstException, Attribute, FixedSizeInteger, Struct, StructField, StructInlinePlaceholder
from catparser.AstPostProcessor import AstPostProcessor


class AstPostProcessorTests(unittest.TestCase):
	# region apply_attributes

	@staticmethod
	def _create_type_descriptors_for_apply_attributes_tests(property_name='sort_key'):
		field_model = StructField(['alpha', Array(['ElementType', 10])])
		field_model.attributes = [Attribute([property_name, 'weight'])]

		return [
			Struct([None, 'ElementType', StructField(['weight', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', field_model]),

			# add a struct with StructInlinePlaceholder to ensure proper field / attribute filtering
			Struct(['inline', 'Inline1', StructField(['counter', FixedSizeInteger('uint8')])]),
			Struct([None, 'Plain2', StructInlinePlaceholder(['Inline1']), StructField(['height', FixedSizeInteger('uint16')])])
		]

	def test_apply_attributes_fails_when_attribute_to_property_mapping_is_unknown(self):
		# Arrange:
		processor = AstPostProcessor(self._create_type_descriptors_for_apply_attributes_tests('sort_direction'))

		# Act:
		with self.assertRaises(AstException):
			processor.apply_attributes()

	def test_apply_attributes_sets_field_type_properties_from_attributes(self):
		# Arrange:
		processor = AstPostProcessor(self._create_type_descriptors_for_apply_attributes_tests())

		# Sanity:
		self.assertEqual(None, processor.type_descriptors[1].fields[0].field_type.sort_key)

		# Act:
		processor.apply_attributes()

		# Assert: sort_key was set
		self.assertEqual('weight', processor.type_descriptors[1].fields[0].field_type.sort_key)

	# endregion

	# region expand_named_inlines

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

	# endregion

	# region expand_unnamed_inlines

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

	def test_expand_unnamed_inlines_expands_unnamed_inlines_with_transitive_factory_type(self):
		# Arrange:
		processor = AstPostProcessor([
			Struct(['abstract', 'Abstract1', StructField(['counter', FixedSizeInteger('uint8')])]),
			Struct(['inline', 'Inline2', StructInlinePlaceholder(['Abstract1']), StructField(['weight', FixedSizeInteger('uint32')])]),
			Struct([None, 'Plain3', StructInlinePlaceholder(['Inline2']), StructField(['height', FixedSizeInteger('uint16')])])
		])

		# Act:
		processor.expand_unnamed_inlines()
		type_descriptors = processor.type_descriptors

		# Assert:
		self.assertEqual(['counter'], [field.name for field in type_descriptors[0].fields])
		self.assertEqual(None, type_descriptors[0].factory_type)

		self.assertEqual(['counter', 'weight', 'height'], [field.name for field in type_descriptors[1].fields])  # Inline2 was expanded
		self.assertEqual('Abstract1', type_descriptors[1].factory_type)  # factory_type is set to abstract grandparent

	def test_expand_unnamed_inlines_expands_unnamed_inlines_with_closest_factory_type(self):
		# Arrange:
		processor = AstPostProcessor([
			Struct(['abstract', 'Abstract1', StructField(['counter', FixedSizeInteger('uint8')])]),
			Struct(['abstract', 'Abstract2', StructInlinePlaceholder(['Abstract1']), StructField(['weight', FixedSizeInteger('uint32')])]),
			Struct([None, 'Plain3', StructInlinePlaceholder(['Abstract2']), StructField(['height', FixedSizeInteger('uint16')])])
		])

		# Act:
		processor.expand_unnamed_inlines()
		type_descriptors = processor.type_descriptors

		# Assert:
		self.assertEqual(['counter'], [field.name for field in type_descriptors[0].fields])
		self.assertEqual(None, type_descriptors[0].factory_type)

		self.assertEqual(['counter', 'weight'], [field.name for field in type_descriptors[1].fields])  # Abstract1 was expanded
		self.assertEqual('Abstract1', type_descriptors[1].factory_type)

		self.assertEqual(['counter', 'weight', 'height'], [field.name for field in type_descriptors[2].fields])  # Abstract2 was expanded
		self.assertEqual('Abstract2', type_descriptors[2].factory_type)  # Abstract2 is prefered to Abstract1

	@staticmethod
	def _create_type_descriptors_for_transitive_attributes_tests(include_b_attributes):
		type_descriptors = [
			Struct(['abstract', 'Abstract1', StructField(['counter', FixedSizeInteger('uint8')])]),
			Struct(['inline', 'Inline2', StructInlinePlaceholder(['Abstract1']), StructField(['weight', FixedSizeInteger('uint32')])]),
			Struct([None, 'Plain3', StructInlinePlaceholder(['Inline2']), StructField(['height', FixedSizeInteger('uint16')])])
		]
		type_descriptors[0].attributes = [Attribute(['a1', 'alpha']), Attribute(['a2', 'beta'])]

		if include_b_attributes:
			type_descriptors[1].attributes = [Attribute(['b1', 'zoo'])]

		type_descriptors[2].attributes = [Attribute(['c1', 'cow'])]
		return type_descriptors

	def test_expand_unnamed_inlines_expands_unnamed_inlines_with_transitive_attributes(self):
		# Arrange:
		processor = AstPostProcessor(self._create_type_descriptors_for_transitive_attributes_tests(True))

		# Act:
		processor.expand_unnamed_inlines()
		type_descriptors = processor.type_descriptors

		# Assert:
		self.assertEqual(['a1', 'a2'], [attribute.name for attribute in type_descriptors[0].attributes])
		self.assertEqual(['c1', 'b1', 'a1', 'a2'], [attribute.name for attribute in type_descriptors[1].attributes])

	def test_expand_unnamed_inlines_expands_unnamed_inlines_with_transitive_attributes_some_levels_missing_attributes(self):
		# Arrange:
		processor = AstPostProcessor(self._create_type_descriptors_for_transitive_attributes_tests(False))

		# Act:
		processor.expand_unnamed_inlines()
		type_descriptors = processor.type_descriptors

		# Assert:
		self.assertEqual(['a1', 'a2'], [attribute.name for attribute in type_descriptors[0].attributes])
		self.assertEqual(['c1', 'a1', 'a2'], [attribute.name for attribute in type_descriptors[1].attributes])

	# endregion
