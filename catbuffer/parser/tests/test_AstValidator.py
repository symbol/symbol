import unittest

from catparser.ast import (
	Alias,
	Array,
	Attribute,
	Conditional,
	Enum,
	EnumValue,
	FixedSizeInteger,
	Struct,
	StructField,
	StructInlinePlaceholder
)
from catparser.AstValidator import AstValidator, ErrorDescriptor

# region ErrorDescriptorTests


class ErrorDescriptorTests(unittest.TestCase):
	def test_can_create_without_field(self):
		# Act:
		error = ErrorDescriptor('this is an error message', 'FooBar')

		# Assert:
		self.assertEqual('[FooBar] this is an error message', str(error))

	def test_can_create_with_single_field(self):
		# Act:
		error = ErrorDescriptor('this is an error message', 'FooBar', 'alpha')

		# Assert:
		self.assertEqual('[FooBar::{ alpha }] this is an error message', str(error))

	def test_can_create_with_multiple_fields(self):
		# Act:
		error = ErrorDescriptor('this is an error message', 'FooBar', ['alpha', 'beta'])

		# Assert:
		self.assertEqual('[FooBar::{ alpha, beta }] this is an error message', str(error))

	def test_equality_is_supported(self):
		# Arrange:
		error = ErrorDescriptor('this is an error message', 'FooBar', 'alpha')

		# Act + Assert:
		self.assertEqual(error, ErrorDescriptor('this is an error message', 'FooBar', 'alpha'))
		self.assertNotEqual(error, ErrorDescriptor('this is another error message', 'FooBar', 'alpha'))
		self.assertNotEqual(error, ErrorDescriptor('this is an error message', 'Foo', 'alpha'))
		self.assertNotEqual(error, ErrorDescriptor('this is an error message', 'FooBar', 'beta'))
		self.assertNotEqual(error, ErrorDescriptor('this is an error message', 'FooBar'))
		self.assertNotEqual(error, str(error))
		self.assertNotEqual(error, None)


# endregion

class AstValidatorTests(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	def _asssert_validate(self, validator, expected_errors, modes=None):
		# Arrange:
		if not modes:
			modes = [AstValidator.Mode.PRE_EXPANSION, AstValidator.Mode.POST_EXPANSION]

		for mode in modes:
			validator.errors = []
			validator.set_validation_mode(mode)

			# Act:
			validator.validate()

			# Assert:
			self.assertEqual(expected_errors, validator.errors)

	# region other types

	def test_can_validate_alias(self):
		# Arrange:
		validator = AstValidator([
			Alias(['AlphaAlias', FixedSizeInteger('uint16')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	# endregion

	# region enum

	def test_can_validate_enum_with_duplicate_child_values(self):
		# Arrange:
		validator = AstValidator([
			Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED1', 0xFF0000]), EnumValue(['RED2', 0xFF0000])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_enum_with_duplicate_child_names(self):
		# Arrange:
		validator = AstValidator([
			Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED', 0xFF0000]), EnumValue(['RED', 0x00FF00])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('duplicate enum values', 'ColorShade', 'RED')
		])

	# endregion

	# region struct - basic

	def test_can_validate_struct_with_unique_field_names(self):
		# Arrange:
		validator = AstValidator([
			Alias(['MyCustomType', FixedSizeInteger('uint16')]),
			Struct([None, 'FooBar', StructField(['alpha', 'MyCustomType']), StructField(['beta', FixedSizeInteger('uint16')])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_with_duplicate_field_names(self):
		# Arrange:
		validator = AstValidator([
			Alias(['MyCustomType', FixedSizeInteger('uint16')]),
			Struct([None, 'FooBar', StructField(['alpha', 'MyCustomType']), StructField(['alpha', FixedSizeInteger('uint16')])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('duplicate struct fields', 'FooBar', 'alpha')
		])

	# endregion

	# region struct - type and inline linking

	def test_cannot_validate_struct_containing_field_with_unknown_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['alpha', 'MyCustomType'])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown type "MyCustomType"', 'FooBar', 'alpha')
		])

	def test_cannot_validate_struct_containing_unnamed_inline_unknown_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructInlinePlaceholder(['MyCustomType'])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown inlined type "MyCustomType"', 'FooBar')
		])

	def test_can_validate_struct_containing_named_inline_field_referencing_inline_type(self):
		# Arrange:
		validator = AstValidator([
			Struct(['inline', 'MyCustomType', StructField(['beta', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', StructField(['alpha', 'MyCustomType'], 'inline')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def _test_cannot_validate_struct_containing_named_inline_field_referencing_type_with_disposition(self, reference_disposition):
		# Arrange:
		validator = AstValidator([
			Struct([reference_disposition, 'MyCustomType', StructField(['beta', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', StructField(['alpha', 'MyCustomType'], 'inline')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('named inline field referencing non inline struct "MyCustomType"', 'FooBar', 'alpha')
		])

	def test_cannot_validate_struct_containing_named_inline_field_referencing_plain_type(self):
		self._test_cannot_validate_struct_containing_named_inline_field_referencing_type_with_disposition(None)

	def test_cannot_validate_struct_containing_named_inline_field_referencing_abstract_type(self):
		self._test_cannot_validate_struct_containing_named_inline_field_referencing_type_with_disposition('abstract')

	def _test_can_validate_struct_containing_unnamed_inline_field_referencing_type_with_disposition(self, reference_disposition):
		# Arrange:
		validator = AstValidator([
			Struct([reference_disposition, 'MyCustomType', StructField(['beta', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', StructInlinePlaceholder(['MyCustomType'])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_can_validate_struct_containing_unnamed_inline_field_referencing_plain_type(self):
		self._test_can_validate_struct_containing_unnamed_inline_field_referencing_type_with_disposition(None)

	def test_can_validate_struct_containing_unnamed_inline_field_referencing_inline_type(self):
		self._test_can_validate_struct_containing_unnamed_inline_field_referencing_type_with_disposition('inline')

	def test_can_validate_struct_containing_unnamed_inline_field_referencing_abstract_type(self):
		self._test_can_validate_struct_containing_unnamed_inline_field_referencing_type_with_disposition('abstract')

	# endregion

	# region struct - array linking

	def test_can_validate_struct_containing_array_with_known_element_type(self):
		# Arrange:
		validator = AstValidator([
			Alias(['ElementType', FixedSizeInteger('uint16')]),
			Struct([None, 'FooBar', StructField(['alpha', Array(['ElementType', 10])])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_array_with_unknown_element_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['alpha', Array(['ElementType', 10])])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown element type "ElementType"', 'FooBar', 'alpha')
		])

	def test_can_validate_struct_containing_array_with_known_size_field(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['sigma', FixedSizeInteger('uint16')]),
				StructField(['alpha', Array([FixedSizeInteger('uint16'), 'sigma'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_array_with_unknown_size_field(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['alpha', Array([FixedSizeInteger('uint16'), 'sigma'])])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown size property "sigma"', 'FooBar', 'alpha')
		])

	def test_can_validate_struct_containing_array_with_known_sort_key_field(self):
		# Arrange:
		array_model = Array(['ElementType', 10])
		array_model.sort_key = ['weight']

		validator = AstValidator([
			Struct([None, 'ElementType', StructField(['weight', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', StructField(['alpha', array_model])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_array_with_unknown_sort_key_field(self):
		# Arrange:
		array_model = Array(['ElementType', 10])
		array_model.sort_key = ['height']

		validator = AstValidator([
			Struct([None, 'ElementType', StructField(['weight', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', StructField(['alpha', array_model])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown sort_key property "height"', 'FooBar', 'alpha')
		])

	def test_cannot_validate_struct_containing_array_with_unknown_element_type_and_unknown_sort_key_field(self):
		# Arrange:
		array_model = Array(['ElementType', 10])
		array_model.sort_key = ['height']

		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['alpha', array_model])])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown element type "ElementType"', 'FooBar', 'alpha'),
			ErrorDescriptor('reference to unknown sort_key property "height"', 'FooBar', 'alpha')
		])

	# endregion

	# region struct - sizeof reference

	def test_can_validate_struct_containing_sizeof_field_with_valid_property_reference_to_struct_type_with_is_size_implicit(self):
		# Arrange:
		other_type_descriptor = Struct([None, 'Other', StructField(['baz', FixedSizeInteger('uint8')])])
		other_type_descriptor.attributes = [Attribute(['is_size_implicit'])]
		validator = AstValidator([
			other_type_descriptor,
			Struct([
				None,
				'FooBar',
				StructField(['r1_size', FixedSizeInteger('uint8'), 'r1'], 'sizeof'),
				StructField(['r1', 'Other'])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_sizeof_field_with_valid_property_reference_to_struct_type_without_is_size_implicit(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'Other', StructField(['baz', FixedSizeInteger('uint8')])]),
			Struct([
				None,
				'FooBar',
				StructField(['r1_size', FixedSizeInteger('uint8'), 'r1'], 'sizeof'),
				StructField(['r1', 'Other'])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('sizeof property references type "Other" without is_size_implicit attribute', 'FooBar', 'r1_size')
		])

	def test_cannot_validate_struct_containing_sizeof_field_with_valid_property_reference_to_fixed_size_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['r1_size', FixedSizeInteger('uint8'), 'r1'], 'sizeof'),
				StructField(['r1', FixedSizeInteger('uint8')])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('sizeof property references fixed size type "uint8"', 'FooBar', 'r1_size')
		])

	def test_cannot_validate_struct_containing_sizeof_field_with_unknown_property_reference(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['r1_size', FixedSizeInteger('uint8'), 'r2'], 'sizeof'),
				StructField(['r1', FixedSizeInteger('uint8')])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown sizeof property "r2"', 'FooBar', 'r1_size')
		])

	# endregion

	# region struct - value consistency

	def test_can_validate_struct_containing_numeric_field_value_for_numeric_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['r1', FixedSizeInteger('uint8'), 101], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_non_numeric_field_value_for_numeric_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['r1', FixedSizeInteger('uint8'), 'RED'], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('field value "RED" is not a valid numeric value', 'FooBar', 'r1')
		])

	def test_can_validate_struct_containing_numeric_field_value_for_numeric_alias_type(self):
		# Arrange:
		validator = AstValidator([
			Alias(['ColorShade', FixedSizeInteger('uint16')]),
			Struct([None, 'FooBar', StructField(['r1', 'ColorShade', 101], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_non_numeric_field_value_for_numeric_alias_type(self):
		# Arrange:
		validator = AstValidator([
			Alias(['ColorShade', FixedSizeInteger('uint16')]),
			Struct([None, 'FooBar', StructField(['r1', 'ColorShade', 'RED'], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('field value "RED" is not a valid numeric value', 'FooBar', 'r1')
		])

	def test_can_validate_struct_containing_enum_value_for_enum_type(self):
		# Arrange:
		validator = AstValidator([
			Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED', 0xFF0000])]),
			Struct([None, 'FooBar', StructField(['r1', 'ColorShade', 'RED'], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_enum_value_for_unknown_enum_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([None, 'FooBar', StructField(['r1', 'ColorShade', 'RED'], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown type "ColorShade"', 'FooBar', 'r1'),
			ErrorDescriptor('field value "RED" is not a valid numeric value', 'FooBar', 'r1')
		])

	def test_cannot_validate_struct_containing_non_enum_value_for_enum_type(self):
		# Arrange:
		validator = AstValidator([
			Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED', 0xFF0000])]),
			Struct([None, 'FooBar', StructField(['r1', 'ColorShade', 'BLUE'], 'reserved')])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('field value "BLUE" is not a valid enum value', 'FooBar', 'r1')
		])

	# endregion

	# region struct - conditional linking

	def test_can_validate_struct_containing_conditional_with_known_condition_field(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['version', FixedSizeInteger('uint8')]),
				StructField(['alpha', FixedSizeInteger('uint32'), Conditional([99, 'equals', 'version'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_conditional_with_unknown_condition_field(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['version', FixedSizeInteger('uint8')]),
				StructField(['alpha', FixedSizeInteger('uint32'), Conditional([99, 'equals', 'trigger'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown condition field "trigger"', 'FooBar', 'alpha'),
		])

	def test_cannot_validate_struct_containing_conditional_with_unknown_condition_type(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['shade', 'ColorShade']),
				StructField(['alpha', FixedSizeInteger('uint32'), Conditional(['RED', 'equals', 'shade'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown type "ColorShade"', 'FooBar', 'shade'),
			ErrorDescriptor('field value "RED" is not a valid numeric value', 'FooBar', 'alpha')
		])

	def test_cannot_validate_struct_containing_conditional_with_invalid_condition_value(self):
		# Arrange:
		validator = AstValidator([
			Struct([
				None,
				'FooBar',
				StructField(['version', FixedSizeInteger('uint8')]),
				StructField(['alpha', FixedSizeInteger('uint32'), Conditional(['RED', 'equals', 'version'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('field value "RED" is not a valid numeric value', 'FooBar', 'alpha')
		])

	def test_can_validate_struct_containing_conditional_with_enum_condition_type(self):
		# Arrange:
		validator = AstValidator([
			Enum(['ColorShade', FixedSizeInteger('uint32'), EnumValue(['RED', 0xFF0000])]),
			Struct([
				None,
				'FooBar',
				StructField(['shade', 'ColorShade']),
				StructField(['alpha', FixedSizeInteger('uint32'), Conditional(['RED', 'equals', 'shade'])])
			])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	# endregion

	# region struct - field attributes

	def test_can_validate_struct_containing_field_with_attribute_mapping_to_field_type_property(self):
		# Arrange:
		field_model = StructField(['alpha', Array(['ElementType', 10])])
		field_model.attributes = [Attribute(['sort_key', 'weight'])]

		validator = AstValidator([
			Struct([None, 'ElementType', StructField(['weight', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', field_model])
		])

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_field_with_attribute_mapping_to_nonexistent_property(self):
		# Arrange:
		field_model = StructField(['alpha', Array(['ElementType', 10])])
		field_model.attributes = [Attribute(['sort_direction', True])]

		validator = AstValidator([
			Struct([None, 'ElementType', StructField(['weight', FixedSizeInteger('uint16')])]),
			Struct([None, 'FooBar', field_model])
		])

		# Act + Assert:
		self._asssert_validate(validator, [
			ErrorDescriptor('inapplicable attribute "sort_direction"', 'FooBar', 'alpha')
		])

	# endregion

	# region struct - struct attributes

	@staticmethod
	def _create_type_descriptors_for_attribute_link_tests(attributes, disposition=None):
		struct_model = Struct([
			disposition,
			'FooBar',
			StructField(['KILO', FixedSizeInteger('uint16'), 1024], 'const'),
			StructField(['weight', FixedSizeInteger('uint16')]),
			StructField(['height', FixedSizeInteger('int8')]),
			StructField(['other', 'MyCustomType'])
		])
		struct_model.attributes = attributes
		return [
			Alias(['MyCustomType', FixedSizeInteger('uint16')]),
			struct_model
		]

	def test_can_validate_struct_containing_size_attribute_referencing_int_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['size', 'height'])]))

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_size_attribute_referencing_non_int_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['size', 'other'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to "size" property "other" has unexpected type', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_cannot_validate_struct_containing_size_attribute_referencing_non_existent_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['size', 'blah'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown "size" property "blah"', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_can_validate_struct_containing_discriminator_attribute_referencing_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['discriminator', 'other'])]))

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_discriminator_attribute_referencing_non_existent_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['discriminator', 'blah'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown "discriminator" property "blah"', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_cannot_validate_struct_containing_discriminator_attribute_referencing_some_existent_properties(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([
			Attribute(['discriminator', 'weight', 'blah', 'other', 'alpha'])
		]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown "discriminator" property "blah"', 'FooBar'),
			ErrorDescriptor('reference to unknown "discriminator" property "alpha"', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_can_validate_struct_containing_initializes_attribute_referencing_known_property_and_const_with_same_type(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['initializes', 'weight', 'KILO'])]))

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_cannot_validate_struct_containing_initializes_attribute_referencing_known_property_and_const_with_different_type(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['initializes', 'height', 'KILO'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('property "height" has initializer "KILO" of different type', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_cannot_validate_struct_containing_initializes_attribute_referencing_non_existent_property(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['initializes', 'blah', 'KILO'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown "intializes" property "blah"', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_cannot_validate_struct_containing_initializes_attribute_referencing_non_existent_const(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests([Attribute(['initializes', 'weight', 'TON'])]))

		# Act + Assert:
		self._asssert_validate(validator, [], [AstValidator.Mode.PRE_EXPANSION])
		self._asssert_validate(validator, [
			ErrorDescriptor('reference to unknown "intializes" property "TON"', 'FooBar')
		], [AstValidator.Mode.POST_EXPANSION])

	def test_can_validate_abstract_struct_containing_initializes_attribute_referencing_non_existent_const(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests(
			[Attribute(['initializes', 'weight', 'TON'])],
			'abstract'))

		# Act + Assert:
		self._asssert_validate(validator, [])

	def test_can_validate_inline_struct_containing_initializes_attribute_referencing_non_existent_const(self):
		# Arrange:
		validator = AstValidator(self._create_type_descriptors_for_attribute_link_tests(
			[Attribute(['initializes', 'weight', 'TON'])],
			'inline'))

		# Act + Assert:
		self._asssert_validate(validator, [])

	# endregion
