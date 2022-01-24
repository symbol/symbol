import unittest

from catparser.ast import Alias, Array, Attribute, FixedSizeInteger, Struct, StructField
from catparser.generators.util import build_factory_map, extend_models

# region build_factory_map


class BuildFactoryMapTests(unittest.TestCase):
	@staticmethod
	def _create_concrete_struct_models(concrete_names, factory_name, factory_attributes):
		models = []
		for name in concrete_names:
			model = Struct([None, name])
			model.attributes = factory_attributes
			model.factory_type = factory_name
			model.fields = [
				StructField(['alpha', 'AlphaType']),
				StructField(['beta', 'BetaType']),
				StructField(['gamma', 'GammaType']),
				StructField(['zeta', 'ZetaType'])
			]
			models.append(model)

		return models

	def test_map_is_empty_when_there_are_no_models(self):
		# Act:
		factory_map = build_factory_map([])

		# Assert:
		self.assertEqual(0, len(factory_map))

	def _assert_model_is_skipped_without_property(self, property_name):
		# Arrange:
		models = self._create_concrete_struct_models(['Foo', 'Bar'], 'Thing', [
			Attribute(['discriminator', 'alpha', 'beta']),
			Attribute(['initializes', 'alpha', 'ALPHAQ']),
			Attribute(['initializes', 'beta', 'BETAZ'])
		])
		setattr(models[0], property_name, None)

		# Act:
		factory_map = build_factory_map(models)

		# Assert:
		self.assertEqual(1, len(factory_map))

		descriptor = factory_map['Thing']
		self.assertEqual(['alpha', 'beta'], descriptor.discriminator_names)
		self.assertEqual(['ALPHAQ', 'BETAZ'], descriptor.discriminator_values)
		self.assertEqual(['AlphaType', 'BetaType'], descriptor.discriminator_types)
		self.assertEqual(['Bar'], [model.name for model in descriptor.children])

	def test_non_structs_are_skipped(self):
		self._assert_model_is_skipped_without_property('display_type')

	def test_structs_without_factory_type_are_skipped(self):
		self._assert_model_is_skipped_without_property('factory_type')

	def _assert_can_aggregate_structs(self, clear_attributes=False):
		# Arrange:
		models = self._create_concrete_struct_models(['Foo', 'Bar'], 'Thing', [
			Attribute(['discriminator', 'alpha', 'beta']),
			Attribute(['initializes', 'alpha', 'ALPHAQ']),
			Attribute(['initializes', 'beta', 'BETAZ'])
		])

		if clear_attributes:
			for model in models[1:]:
				model.attributes = None

		# Act:
		factory_map = build_factory_map(models)

		# Assert:
		self.assertEqual(1, len(factory_map))

		descriptor = factory_map['Thing']
		self.assertEqual(['alpha', 'beta'], descriptor.discriminator_names)
		self.assertEqual(['ALPHAQ', 'BETAZ'], descriptor.discriminator_values)
		self.assertEqual(['AlphaType', 'BetaType'], descriptor.discriminator_types)
		self.assertEqual(['Foo', 'Bar'], [model.name for model in descriptor.children])

	def test_map_aggregates_all_structs_with_same_factory_type(self):
		self._assert_can_aggregate_structs()

	def test_factory_descriptors_are_populated_with_attributes_from_first_matching_concrete_struct(self):
		self._assert_can_aggregate_structs(clear_attributes=True)

	def test_discriminator_ordering_takes_precedence_over_initializer_ordering(self):
		# Arrange:
		models = self._create_concrete_struct_models(['Foo', 'Bar'], 'Thing', [
			Attribute(['discriminator', 'beta', 'alpha']),
			Attribute(['initializes', 'alpha', 'ALPHAQ']),
			Attribute(['initializes', 'beta', 'BETAZ'])
		])

		# Act:
		factory_map = build_factory_map(models)

		# Assert: beta is first discriminator and alpha is second
		self.assertEqual(1, len(factory_map))

		descriptor = factory_map['Thing']
		self.assertEqual(['beta', 'alpha'], descriptor.discriminator_names)
		self.assertEqual(['BETAZ', 'ALPHAQ'], descriptor.discriminator_values)
		self.assertEqual(['BetaType', 'AlphaType'], descriptor.discriminator_types)
		self.assertEqual(['Foo', 'Bar'], [model.name for model in descriptor.children])

	def test_map_can_contain_multiple_values(self):
		# Arrange:
		thing_models = self._create_concrete_struct_models(['Foo', 'Bar'], 'Thing', [
			Attribute(['discriminator', 'beta', 'alpha']),
			Attribute(['initializes', 'alpha', 'ALPHAQ']),
			Attribute(['initializes', 'beta', 'BETAZ'])
		])

		other_models = self._create_concrete_struct_models(['Cat', 'Robot', 'Lumberjack'], 'Other', [
			Attribute(['discriminator', 'gamma', 'zeta']),
			Attribute(['initializes', 'gamma', 'GGG']),
			Attribute(['initializes', 'zeta', 'ZZZ'])
		])

		# Act:
		factory_map = build_factory_map(thing_models + other_models)

		# Assert:
		self.assertEqual(2, len(factory_map))

		descriptor = factory_map['Thing']
		self.assertEqual(['beta', 'alpha'], descriptor.discriminator_names)
		self.assertEqual(['BETAZ', 'ALPHAQ'], descriptor.discriminator_values)
		self.assertEqual(['BetaType', 'AlphaType'], descriptor.discriminator_types)
		self.assertEqual(['Foo', 'Bar'], [model.name for model in descriptor.children])

		descriptor = factory_map['Other']
		self.assertEqual(['gamma', 'zeta'], descriptor.discriminator_names)
		self.assertEqual(['GGG', 'ZZZ'], descriptor.discriminator_values)
		self.assertEqual(['GammaType', 'ZetaType'], descriptor.discriminator_types)
		self.assertEqual(['Cat', 'Robot', 'Lumberjack'], [model.name for model in descriptor.children])


# endregion

# region extend_models

class ExtendModelsTests(unittest.TestCase):
	@staticmethod
	def _printer_factory(type_model, name, is_pod):
		pod_descriptor = 'pod' if is_pod else 'custom'
		return f'{type_model.name} {name} ({pod_descriptor})'

	def _assert_field_extensions(self, field_extensions, name, printer_name, **kwargs):
		self.assertEqual(name, field_extensions.type_model.name)
		self.assertEqual(printer_name, field_extensions.printer)

		self.assertEqual(kwargs.get('is_contents_abstract', False), field_extensions.is_contents_abstract)

		if 'bound_field_name' in kwargs:
			self.assertEqual(kwargs['bound_field_name'], field_extensions.bound_field.name)
		else:
			self.assertEqual(None, field_extensions.bound_field)

		if 'size_field_names' in kwargs:
			self.assertEqual(kwargs['size_field_names'], [size_field.name for size_field in field_extensions.size_fields])
		else:
			self.assertEqual([], field_extensions.size_fields)

	def test_extends_pod_type_field(self):
		# Arrange:
		models = [Struct([None, 'Foo', StructField(['beta', FixedSizeInteger('uint16')])])]

		# Act:
		extend_models(models, self._printer_factory)

		# Assert:
		self._assert_field_extensions(models[0].fields[0].extensions, 'beta', 'beta beta (pod)')

	def test_extends_custom_type_field(self):
		# Arrange:
		models = [
			Alias(['Bar', FixedSizeInteger('uint16')]),
			Struct([None, 'Foo', StructField(['beta', 'Bar'])])
		]

		# Act:
		extend_models(models, self._printer_factory)

		# Assert:
		self._assert_field_extensions(models[1].fields[0].extensions, 'Bar', 'Bar beta (custom)')

	def _assert_extends_array_type_field(self, disposition, is_contents_abstract):
		# Arrange:
		models = [
			Struct([disposition, 'Bar']),
			Struct([None, 'Foo', StructField(['beta', Array(['Bar', 8])])])
		]

		# Act:
		extend_models(models, self._printer_factory)

		# Assert:
		self._assert_field_extensions(models[1].fields[0].extensions, 'beta', 'beta beta (pod)', is_contents_abstract=is_contents_abstract)

	def test_extends_concrete_array_type_field(self):
		self._assert_extends_array_type_field(None, False)

	def test_extends_abstract_array_type_field(self):
		self._assert_extends_array_type_field('abstract', True)

	def test_extends_array_type_field_with_bound_size(self):
		# Arrange:
		models = [
			Alias(['Bar', FixedSizeInteger('uint16')]),
			Struct([
				None,
				'Foo',
				StructField(['beta', Array(['Bar', 'beta_size'])]),
				StructField(['beta_size', FixedSizeInteger('uint16')])
			])
		]

		# Act:
		extend_models(models, self._printer_factory)

		# Assert:
		self._assert_field_extensions(models[1].fields[0].extensions, 'beta', 'beta beta (pod)')
		self._assert_field_extensions(models[1].fields[1].extensions, 'beta_size', 'beta_size beta_size (pod)', bound_field_name='beta')

	def test_extends_size_reference_field_with_bound_size(self):
		# Arrange:
		models = [
			Alias(['Bar', FixedSizeInteger('uint16')]),
			Struct([
				None,
				'Foo',
				StructField(['beta', 'Bar']),
				StructField(['beta_size', FixedSizeInteger('uint16'), 'beta'], 'sizeof')
			])
		]

		# Act:
		extend_models(models, self._printer_factory)

		# Assert:
		self._assert_field_extensions(models[1].fields[0].extensions, 'Bar', 'Bar beta (custom)', size_field_names=['beta_size'])
		self._assert_field_extensions(models[1].fields[1].extensions, 'beta_size', 'beta_size beta_size (pod)', bound_field_name='beta')


# endregion
