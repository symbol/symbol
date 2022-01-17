from enum import Enum

from .ast import Array, Conditional, Enum, FixedSizeInteger, Struct, StructInlinePlaceholder


class ErrorDescriptor:
	"""Describes an AST validation error."""

	def __init__(self, message, typename, field_names=None):
		self.message = message
		self.typename = typename
		self.field_names = [field_names] if isinstance(field_names, str) else field_names

	def __eq__(self, rhs):
		return isinstance(rhs, ErrorDescriptor) and str(self) == str(rhs)

	def __repr__(self):
		if self.field_names:
			joined_field_names = ', '.join(self.field_names)
			return f'[{self.typename}::{{ {joined_field_names} }}] {self.message}'

		return f'[{self.typename}] {self.message}'


class AstValidator:
	"""Validates AST type descriptors."""

	class Mode(Enum):
		"""Validation mode."""

		PRE_EXPANSION = 1
		POST_EXPANSION = 2

	def __init__(self, type_descriptors):
		self.raw_type_descriptors = type_descriptors
		self.type_descriptor_map = {model.name: model for model in self.raw_type_descriptors}

		self.mode = self.Mode.PRE_EXPANSION
		self.errors = []

	def set_validation_mode(self, mode):
		"""Sets the validation mode."""
		self.mode = mode

	def validate(self):
		"""Validates all types for correctness."""

		for _, model in self.type_descriptor_map.items():
			if isinstance(model, Enum):
				self._validate_enum(model)

			if isinstance(model, Struct):
				self._validate_struct(model)

	def _validate_enum(self, model):
		duplicate_names = self._find_duplicate_names(model.values)
		if duplicate_names:
			self.errors.append(ErrorDescriptor('duplicate enum values', model.name, duplicate_names))

	def _validate_struct(self, model):
		duplicate_names = self._find_duplicate_names(model.fields)
		if duplicate_names:
			self.errors.append(ErrorDescriptor('duplicate struct fields', model.name, duplicate_names))

		field_map = {field.name: field for field in model.fields if hasattr(field, 'name')}
		for field in model.fields:
			def create_error_descriptor(message, error_field=field):
				return ErrorDescriptor(message, model.name, error_field.name)

			if isinstance(field, StructInlinePlaceholder):
				self._validate_unnamed_inline(field, lambda message: ErrorDescriptor(message, model.name))
			else:
				self._validate_struct_field(field, field_map, create_error_descriptor)

		if self.Mode.PRE_EXPANSION != self.mode:
			self._check_struct_attributes(model, field_map)

	def _validate_unnamed_inline(self, field, create_error_descriptor):
		if not self._is_known_type(field.inlined_typename):
			self.errors.append(create_error_descriptor(f'reference to unknown inlined type "{field.inlined_typename}"'))
		else:
			# all dispositions are allowed as unnamed inline
			pass

	def _validate_struct_field(self, field, field_map, create_error_descriptor):
		if not self._is_known_type(field.field_type):
			self.errors.append(create_error_descriptor(f'reference to unknown type "{field.field_type}"'))
		else:
			if 'inline' == field.disposition and 'inline' != self.type_descriptor_map[field.field_type].disposition:
				self.errors.append(create_error_descriptor(f'named inline field referencing non inline struct "{field.field_type}"'))

		if isinstance(field.field_type, Array):
			self._validate_array(field.field_type, field_map, create_error_descriptor)

		if field.value is not None:
			if 'sizeof' == field.disposition:
				self._validate_sizeof(field, field_map, create_error_descriptor)
			elif isinstance(field.value, Conditional):
				self._validate_conditional(field, field_map, create_error_descriptor)
			else:
				self._validate_in_range(field.field_type, field.value, create_error_descriptor)

		if field.attributes:
			for attribute in field.attributes:
				if not hasattr(field.field_type, attribute.name):
					self.errors.append(create_error_descriptor(f'inapplicable attribute "{attribute.name}"'))

	def _validate_array(self, field_type, field_map, create_error_descriptor):
		element_type = field_type.element_type
		is_sort_key_valid = True
		sort_key = field_type.sort_key
		if not self._is_known_type(element_type):
			self.errors.append(create_error_descriptor(f'reference to unknown element type "{element_type}"'))
			is_sort_key_valid = not sort_key
		else:
			is_sort_key_valid = not sort_key or any(
				sort_key == element_field.name for element_field in self.type_descriptor_map[element_type].fields
			)

		if not is_sort_key_valid:
			self.errors.append(create_error_descriptor(f'reference to unknown sort_key property "{sort_key}"'))

		size = field_type.size
		if isinstance(size, str) and size not in field_map:
			self.errors.append(create_error_descriptor(f'reference to unknown size property "{size}"'))

	def _validate_sizeof(self, field, field_map, create_error_descriptor):
		if field.value not in field_map:
			self.errors.append(create_error_descriptor(f'reference to unknown sizeof property "{field.value}"'))
			return

		reference_typename = field_map[field.value].field_type
		reference_type = None if not isinstance(reference_typename, str) else self.type_descriptor_map[reference_typename]
		if not isinstance(reference_type, Struct):
			self.errors.append(create_error_descriptor(f'sizeof property references fixed size type "{reference_typename}"'))
		elif not reference_type.is_size_implicit:
			message = f'sizeof property references type "{reference_typename}" without is_size_implicit attribute'
			self.errors.append(create_error_descriptor(message))

	def _validate_conditional(self, field, field_map, create_error_descriptor):
		linked_field_name = field.value.linked_field_name

		if linked_field_name not in field_map:
			self.errors.append(create_error_descriptor(f'reference to unknown condition field "{linked_field_name}"'))
		else:
			self._validate_in_range(field_map[linked_field_name].field_type, field.value.value, create_error_descriptor)

	def _validate_in_range(self, value_type, value, create_error_descriptor):
		if isinstance(value_type, str):
			if self._is_known_type(value_type):
				value_type = self.type_descriptor_map[value_type]

				if isinstance(value_type, Enum):
					if not any(value == enum_value.name for enum_value in value_type.values):
						self.errors.append(create_error_descriptor(f'field value "{value}" is not a valid enum value'))

					return

		if not isinstance(value, int):
			self.errors.append(create_error_descriptor(f'field value "{value}" is not a valid numeric value'))

	def _check_struct_attributes(self, model, field_map):
		if self._check_known_field(model, field_map, 'size'):
			if not isinstance(field_map[model.size].field_type, FixedSizeInteger):
				self.errors.append(ErrorDescriptor(f'reference to "size" property "{model.size}" has unexpected type', model.name))

		self._check_known_field(model, field_map, 'discriminator', True)

		if not model.initializers:
			return

		for initializer in model.initializers:
			is_valid = True
			is_concrete = model.disposition not in ('abstract', 'inline')
			for property_name, raise_error in [(initializer.target_property_name, True), (initializer.value, is_concrete)]:
				if property_name not in field_map:
					is_valid = False
					if raise_error:
						self.errors.append(ErrorDescriptor(
							f'reference to unknown "intializes" property "{property_name}"',
							model.name))

			if not is_valid:
				continue

			# str compare is good enough as it lets us differentiate among subtypes of things like FixedSizeInteger
			if str(field_map[initializer.target_property_name].field_type) != str(field_map[initializer.value].field_type):
				self.errors.append(ErrorDescriptor(
					f'property "{initializer.target_property_name}" has initializer "{initializer.value}" of different type',
					model.name))

	def _check_known_field(self, model, field_map, property_name, multi_value=False):
		values = getattr(model, property_name)
		if not values:
			return False

		if not multi_value:
			values = [values]

		has_error = False
		for value in values:
			if value not in field_map:
				self.errors.append(ErrorDescriptor(f'reference to unknown "{property_name}" property "{value}"', model.name))
				has_error = True

		return not has_error

	def _is_known_type(self, typename):
		return not isinstance(typename, str) or typename in self.type_descriptor_map

	@staticmethod
	def _find_duplicate_names(items):
		unique_names = set()
		duplicate_names = set()
		for item in filter(lambda item: hasattr(item, 'name'), items):
			(unique_names if item.name not in unique_names else duplicate_names).add(item.name)

		return duplicate_names
