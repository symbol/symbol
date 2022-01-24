from catparser.DisplayType import DisplayType

from .name_formatting import fix_name, fix_size_name, lang_field_name, underline_name


def js_bool(value):
	return 'true' if value else 'false'


class Printer:
	def __init__(self, descriptor, name):
		self.descriptor = descriptor
		# printer.name is 'fixed' field name
		self.name = fix_name(lang_field_name(name or underline_name(self.descriptor.name)))


class IntPrinter(Printer):
	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = None

	@staticmethod
	def get_type():
		return 'int'

	def get_default_value(self):
		return '0n' if 8 == self.descriptor.size else '0'

	def get_size(self):
		return self.descriptor.size

	def load(self, buffer_name='byteArray'):
		data_size = self.get_size()
		return f'converter.bytesToInt({buffer_name}, {data_size}, {js_bool(not self.descriptor.is_unsigned)})'

	def advancement_size(self):
		return self.get_size()

	def store(self, field_name):
		return f'converter.intToBytes({field_name}, {self.get_size()}, {js_bool(not self.descriptor.is_unsigned)})'

	@staticmethod
	def assign(value):
		return str(value)

	@staticmethod
	def to_string(field_name):
		return f'\'0x\'.concat({field_name}.toString(16))'


class TypedArrayPrinter(Printer):
	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = f'array[{self.descriptor.field_type.element_type}]'

	def get_type(self):
		return f'List[{self.descriptor.field_type.element_type}]'

	@staticmethod
	def get_default_value():
		return '[]'

	def get_size(self):
		if self.descriptor.field_type.is_byte_constrained:
			# note: use actual `.size` field
			alignment = self.descriptor.field_type.alignment
			return f'this.{self.name}.map(e => arrayHelpers.alignUp(e.size, {alignment})).reduce((a, b) => a + b, 0)'

		return f'this.{self.name}.map(e => e.size).reduce((a, b) => a + b, 0)'

	def load(self, buffer_name):
		del buffer_name

		if self.descriptor.field_type.is_byte_constrained:
			# use either type name or if it's an abstract type use a factory instead
			factory_name = self.descriptor.field_type.element_type
			if self.descriptor.extensions.is_contents_abstract:
				factory_name = f'{self.descriptor.field_type.element_type}Factory'

			data_size = lang_field_name(self.descriptor.size)
			alignment = self.descriptor.field_type.alignment
			buffer_view = f'view.window({data_size})'
			return f'arrayHelpers.readVariableSizeElements({buffer_view}, {factory_name}, {alignment})'

		if self.descriptor.field_type.is_expandable:
			return f'arrayHelpers.readArray(view.buffer, {self.descriptor.field_type.element_type})'

		args = [
			'view.buffer',
			self.descriptor.field_type.element_type,
			lang_field_name(str(self.descriptor.size)),
		]
		if self.descriptor.field_type.sort_key:
			accessor = f'e => e.{lang_field_name(self.descriptor.field_type.sort_key)}.value'
			args.append(accessor)

		args_str = ', '.join(args)
		return f'arrayHelpers.readArrayCount({args_str})'

	def advancement_size(self):
		if self.descriptor.field_type.is_byte_constrained:
			return lang_field_name(str(self.descriptor.size))

		return f'{self.name}.map(e => e.size).reduce((a, b) => a + b, 0)'

	def store(self, field_name, buffer_name):
		if self.descriptor.field_type.is_byte_constrained:
			alignment = self.descriptor.field_type.alignment
			return f'arrayHelpers.writeVariableSizeElements({buffer_name}, {field_name}, {alignment})'

		if self.descriptor.field_type.is_expandable:
			return f'arrayHelpers.writeArray({buffer_name}, {field_name})'

		args = [buffer_name, field_name]
		size = self.descriptor.size
		if not isinstance(size, str):
			args.append(str(size))

		if self.descriptor.field_type.sort_key:
			accessor = f'e => e.{lang_field_name(self.descriptor.field_type.sort_key)}.value'
			args.append(accessor)

		args_str = ', '.join(args)
		if isinstance(size, str):
			return f'arrayHelpers.writeArray({args_str})'

		return f'arrayHelpers.writeArrayCount({args_str})'

	@staticmethod
	def to_string(field_name):
		return f'{field_name}.map(e => e.toString()).join(\',\')'


class ArrayPrinter(Printer):
	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = 'bytes_array'

	@staticmethod
	def get_type():
		return 'bytes'

	def get_default_value(self):
		size = self.descriptor.size
		if isinstance(size, str):
			return 'new Uint8Array()'

		return f'new Uint8Array({self.get_size()})'

	def get_size(self):
		size = self.descriptor.size
		if isinstance(size, str):
			return f'this._{self.name}.length'

		return size

	def load(self, buffer_name='byteArray'):
		return f'new Uint8Array({buffer_name}.buffer, {buffer_name}.byteOffset, {self.advancement_size()})'

	def advancement_size(self):
		# like get_size() but without self prefix, as this refers to local method field
		if not isinstance(self.descriptor.size, str):
			return self.descriptor.size

		return fix_size_name(lang_field_name(self.descriptor.size))

	@staticmethod
	def store(field_name):
		return field_name

	@staticmethod
	def to_string(field_name):
		return f'converter.uint8ToHex({field_name})'


class BuiltinPrinter(Printer):
	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = {
			DisplayType.INTEGER: 'pod:',
			DisplayType.BYTE_ARRAY: 'pod:',
			DisplayType.TYPED_ARRAY: 'pod:',
			DisplayType.ENUM: 'enum:',
			DisplayType.STRUCT: 'struct:'
		}[self.descriptor.display_type]

		self.type_hint += self.descriptor.name

	def get_type(self):
		return self.descriptor.name

	def get_default_value(self):
		if DisplayType.ENUM == self.descriptor.display_type:
			first_enum_value_name = self.descriptor.values[0].name
			return f'{self.get_type()}.{first_enum_value_name}'

		return f'new {self.get_type()}()'

	def get_size(self):
		return f'this.{self.name}.size'

	def load(self, buffer_name='view.buffer'):
		if DisplayType.STRUCT == self.descriptor.display_type and self.descriptor.is_abstract:
			# HACK: factories use this printers as well, ignore them
			if 'parent' != self.name:
				factory_name = self.get_type() + 'Factory'
				return f'{factory_name}.deserialize({buffer_name})'

		return f'{self.get_type()}.deserialize({buffer_name})'

	def advancement_size(self):
		return f'{self.name}.size'

	@staticmethod
	def store(field_name):
		return f'{field_name}.serialize()'

	def assign(self, value):
		return f'{self.get_type()}.{value}'

	@staticmethod
	def to_string(field_name):
		return f'{field_name}.toString()'


def create_pod_printer(descriptor, name=None):
	display_type = descriptor.display_type
	if DisplayType.INTEGER == display_type:
		PrinterType = IntPrinter
	elif DisplayType.BYTE_ARRAY == display_type:
		PrinterType = ArrayPrinter
	else:
		PrinterType = TypedArrayPrinter

	return PrinterType(descriptor, name)
