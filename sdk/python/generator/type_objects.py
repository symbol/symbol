import re

from catparser.ast import Alias, StructField


class BaseObject:
	# pylint: disable=too-many-instance-attributes

	CAMEL_CASE_PATTERN = re.compile(r'(?<!^)(?=[A-Z])')

	def __init__(self, base_typename, ast_model):
		self.base_typename = base_typename
		self.is_struct = 'struct' == base_typename
		self.is_array = 'array' == base_typename
		self.is_int = 'int' == base_typename
		self.is_enum = 'enum' == base_typename

		self.is_builtin = True
		self.sizeof_value = ast_model.value if hasattr(ast_model, 'disposition') and 'sizeof' == ast_model.disposition else None

		self.ast_model = ast_model
		self.printer = None

		self.typename = self.ast_model.name if hasattr(self.ast_model, 'name') else str(self.ast_model)
		self.field_name = BaseObject.CAMEL_CASE_PATTERN.sub('_', self.typename).lower()

	def set_printer(self, printer):
		self.printer = printer

	@property
	def underlined_name(self):
		return self.field_name

	@property
	def size(self):
		if isinstance(self.ast_model, Alias):
			return self.ast_model.linked_type.size
		if isinstance(self.ast_model, StructField):
			return self.ast_model.field_type.size
		return self.ast_model.size


class IntObject(BaseObject):
	def __init__(self, ast_model):
		super().__init__('int', ast_model)


class ArrayObject(BaseObject):
	def __init__(self, ast_model):
		super().__init__('array', ast_model)
		self.element_type = None

	@property
	def is_sized(self):
		return 'array sized' == self.ast_model.field_type.disposition

	@property
	def is_fill(self):
		return 'array fill' == self.ast_model.field_type.disposition

	def get_type(self):
		return self.ast_model.field_type if isinstance(self.ast_model.field_type, str) else self.ast_model.field_type.element_type


class EnumObject(BaseObject):
	def __init__(self, ast_model):
		super().__init__('enum', ast_model)

	@property
	def underlined_name(self):
		return 'value'

	@property
	def values(self):
		return self.ast_model.values

	@property
	def size(self):
		return self.ast_model.base.size


class StructObject(BaseObject):
	def __init__(self, ast_model):
		super().__init__('struct', ast_model)
		self.layout = []
		self.dynamic_size = self.ast_model.size
		self.name_to_index = {}
		self.is_abstract = 'abstract' == self.ast_model.disposition

	def add_field(self, field):
		self.name_to_index[field.original_field_name] = len(self.layout)
		self.layout.append(field)

	def get_field_by_name(self, field_name):
		index = self.name_to_index[field_name]
		return self.layout[index]
