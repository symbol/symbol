from catparser.DisplayType import DisplayType

from .name_formatting import fix_name, fix_size_name, lang_field_name, underline_name


def js_bool(value):
	return 'true' if value else 'false'


class Printer:
	def __init__(self, descriptor, name):
		self.descriptor = descriptor
		# printer.name is 'fixed' field name
		self.name = fix_name(lang_field_name(name or underline_name(self.descriptor.name)))

	def sort(self, _field_name):  # pylint: disable=no-self-use
		return None


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

	def load(self, buffer_name='byteArray', is_aligned=False):
		data_size = self.get_size()
		arguments = f'{buffer_name}, {data_size}, {js_bool(not self.descriptor.is_unsigned)}'
		qualifier = '' if data_size < 8 else 'Big'

		# is_aligned - handles both generation of deserializeAligned for pod and enum types and generation of fields within struct
		if is_aligned:
			return f'converter.bytesTo{qualifier}Int({arguments})'

		return f'converter.bytesTo{qualifier}IntUnaligned({arguments})'

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

	@property
	def is_variable_size(self):
		# note: self.descriptor.field_type.alignment condition at the end is needed to filter out receipts
		descriptor = self.descriptor
		return (descriptor.field_type.is_byte_constrained or descriptor.extensions.is_contents_abstract) and descriptor.field_type.alignment

	def get_size(self):
		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip_last_element_padding = js_bool(not self.descriptor.field_type.is_last_element_padded)
			return f'arrayHelpers.size(this.{self.name}, {alignment}, {skip_last_element_padding})'

		return f'arrayHelpers.size(this.{self.name})'

	def _get_sort_comparer(self, variable_name):
		sort_key = lang_field_name(self.descriptor.field_type.sort_key)
		comparer = f'({variable_name}.{sort_key}.comparer ? {variable_name}.{sort_key}.comparer() : {variable_name}.{sort_key}.value)'
		return comparer

	def _get_sort_accessor(self):
		accessor = f'e => ({self._get_sort_comparer("e")})'
		return accessor

	def load(self, buffer_name, is_aligned):
		del buffer_name
		del is_aligned
		element_type = self.descriptor.field_type.element_type

		# use either type name or if it's an abstract type use a factory instead
		if self.descriptor.extensions.is_contents_abstract:
			element_type = f'{element_type}Factory'

		if self.is_variable_size:
			buffer_view = None
			if self.descriptor.field_type.is_expandable:
				buffer_view = 'view.buffer'
			else:
				data_size = lang_field_name(self.descriptor.size)
				buffer_view = f'view.window({data_size})'

			alignment = self.descriptor.field_type.alignment
			skip_last_element_padding = js_bool(not self.descriptor.field_type.is_last_element_padded)
			return f'arrayHelpers.readVariableSizeElements({buffer_view}, {element_type}, {alignment}, {skip_last_element_padding})'

		if self.descriptor.field_type.is_expandable:
			return f'arrayHelpers.readArray(view.buffer, {element_type})'

		args = [
			'view.buffer',
			element_type,
			lang_field_name(str(self.descriptor.size)),
		]
		if self.descriptor.field_type.sort_key:
			args.append(self._get_sort_accessor())

		args_str = ', '.join(args)
		return f'arrayHelpers.readArrayCount({args_str})'

	def advancement_size(self):
		if self.descriptor.field_type.is_byte_constrained:
			return lang_field_name(str(self.descriptor.size))

		alignment = self.descriptor.field_type.alignment
		if alignment:
			skip_last_element_padding = js_bool(not self.descriptor.field_type.is_last_element_padded)
			return f'arrayHelpers.size({self.name}, {alignment}, {skip_last_element_padding})'

		return f'arrayHelpers.size({self.name})'

	def store(self, field_name, buffer_name):
		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip_last_element_padding = js_bool(not self.descriptor.field_type.is_last_element_padded)
			return f'arrayHelpers.writeVariableSizeElements({buffer_name}, {field_name}, {alignment}, {skip_last_element_padding})'

		if self.descriptor.field_type.is_expandable:
			return f'arrayHelpers.writeArray({buffer_name}, {field_name})'

		args = [buffer_name, field_name]
		size = self.descriptor.size
		if not isinstance(size, str):
			args.append(str(size))

		if self.descriptor.field_type.sort_key:
			args.append(self._get_sort_accessor())

		args_str = ', '.join(args)
		if isinstance(size, str):
			return f'arrayHelpers.writeArray({args_str})'

		return f'arrayHelpers.writeArrayCount({args_str})'

	def sort(self, field_name):
		if not self.descriptor.field_type.sort_key:
			return None

		body = f'{field_name} = {field_name}.sort((lhs, rhs) => arrayHelpers.deepCompare(\n'
		body += f'\t{self._get_sort_comparer("lhs")},\n'
		body += f'\t{self._get_sort_comparer("rhs")}\n'
		body += '));'
		return body

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

	def load(self, buffer_name='byteArray', is_aligned=False):
		del is_aligned
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

	def load(self, buffer_name='view.buffer', is_aligned=False):
		display_type = self.descriptor.display_type
		if DisplayType.STRUCT == display_type and self.descriptor.is_abstract:
			# HACK: factories use this printers as well, ignore them
			if 'parent' != self.name:
				factory_name = self.get_type() + 'Factory'
				return f'{factory_name}.deserialize({buffer_name})'

		if is_aligned and display_type in (DisplayType.INTEGER, DisplayType.ENUM):
			return f'{self.get_type()}.deserializeAligned({buffer_name})'

		return f'{self.get_type()}.deserialize({buffer_name})'

	def advancement_size(self):
		return f'{self.name}.size'

	@staticmethod
	def store(field_name):
		return f'{field_name}.serialize()'

	def sort(self, field_name):
		return f'{field_name}.sort();' if DisplayType.STRUCT == self.descriptor.display_type else None

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
