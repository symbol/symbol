from catparser.DisplayType import DisplayType

from .name_formatting import fix_name, fix_size_name, underline_name


class Printer:
	def __init__(self, descriptor, name):
		self.descriptor = descriptor
		# printer.name is 'fixed' field name
		self.name = fix_name(name or underline_name(self.descriptor.name))

	def sort(self, _field_name):  # pylint: disable=no-self-use
		return None


class IntPrinter(Printer):
	def __init__(self, descriptor, name=None):
		super().__init__(descriptor, name)
		self.type_hint = None

	@staticmethod
	def get_type():
		return 'int'

	@staticmethod
	def get_default_value():
		return '0'

	def get_size(self):
		return self.descriptor.size

	def load(self):
		data_size = self.get_size()
		return f'int.from_bytes(buffer[:{data_size}], byteorder=\'little\', signed={not self.descriptor.is_unsigned})'

	def advancement_size(self):
		return self.get_size()

	def store(self, field_name):
		return f'{field_name}.to_bytes({self.get_size()}, byteorder=\'little\', signed={not self.descriptor.is_unsigned})'

	@staticmethod
	def assign(value):
		return str(value)

	@staticmethod
	def to_string(field_name):
		return f'0x{{{field_name}:X}}'


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
			skip_last_element_padding = not self.descriptor.field_type.is_last_element_padded
			return f'ArrayHelpers.size(self.{self.name}, {alignment}, skip_last_element_padding={skip_last_element_padding})'

		return f'ArrayHelpers.size(self.{self.name})'

	def _get_sort_accessor(self):
		sort_key = self.descriptor.field_type.sort_key
		accessor = f'lambda e: e.{sort_key}.comparer() if hasattr(e.{sort_key}, \'comparer\') else e.{sort_key}'
		return accessor

	def load(self):
		element_type = self.descriptor.field_type.element_type

		# use either type name or if it's an abstract type use a factory instead
		if self.descriptor.extensions.is_contents_abstract:
			element_type = f'{element_type}Factory'

		if self.is_variable_size:
			buffer = f'buffer[:{self.descriptor.size}]'
			if self.descriptor.field_type.is_expandable:
				buffer = 'buffer'

			alignment = self.descriptor.field_type.alignment
			skip_last_element_padding = f'skip_last_element_padding={not self.descriptor.field_type.is_last_element_padded}'
			return f'ArrayHelpers.read_variable_size_elements({buffer}, {element_type}, {alignment}, {skip_last_element_padding})'

		if self.descriptor.field_type.is_expandable:
			return f'ArrayHelpers.read_array(buffer, {element_type})'

		args = [
			'buffer',
			element_type,
			str(self.descriptor.size),
		]
		if self.descriptor.field_type.sort_key:
			args.append(self._get_sort_accessor())

		args_str = ', '.join(args)
		return f'ArrayHelpers.read_array_count({args_str})'

	def advancement_size(self):
		if self.descriptor.field_type.is_byte_constrained:
			return str(self.descriptor.size)

		alignment = self.descriptor.field_type.alignment
		if alignment:
			return f'ArrayHelpers.size({self.name}, {alignment}, skip_last_element_padding={not self.descriptor.field_type.is_last_element_padded})'

		return f'ArrayHelpers.size({self.name})'

	def store(self, field_name):
		if self.is_variable_size:
			alignment = self.descriptor.field_type.alignment
			skip_last_element_padding = not self.descriptor.field_type.is_last_element_padded
			return f'ArrayHelpers.write_variable_size_elements({field_name}, {alignment}, skip_last_element_padding={skip_last_element_padding})'

		if self.descriptor.field_type.is_expandable:
			return f'ArrayHelpers.write_array({field_name})'

		args = [field_name]
		size = self.descriptor.size
		if not isinstance(size, str):
			args.append(str(size))

		if self.descriptor.field_type.sort_key:
			args.append(self._get_sort_accessor())

		args_str = ', '.join(args)
		if isinstance(size, str):
			return f'ArrayHelpers.write_array({args_str})'

		return f'ArrayHelpers.write_array_count({args_str})'

	def sort(self, field_name):
		if not self.descriptor.field_type.sort_key:
			return None

		return f'{field_name} = sorted({field_name}, key={self._get_sort_accessor()})'

	@staticmethod
	def to_string(field_name):
		return f'list(map(str, {field_name}))'


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
			return 'bytes()'

		return f'bytes({self.get_size()})'

	def get_size(self):
		size = self.descriptor.size
		if isinstance(size, str):
			return f'len(self._{self.name})'

		return size

	def load(self):
		return f'ArrayHelpers.get_bytes(buffer, {self.advancement_size()})'

	def advancement_size(self):
		# like get_size() but without self prefix, as this refers to local method field
		return fix_size_name(self.descriptor.size)

	@staticmethod
	def store(field_name):
		return field_name

	@staticmethod
	def to_string(field_name):
		return f'hexlify({field_name}).decode("utf8")'


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

		return f'{self.get_type()}()'

	def get_size(self):
		return f'self.{self.name}.size'

	def load(self, buffer_name='buffer'):
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

	def sort(self, field_name):
		return f'{field_name}.sort()' if DisplayType.STRUCT == self.descriptor.display_type else None

	def assign(self, value):
		return f'{self.get_type()}.{value}'

	@staticmethod
	def to_string(field_name):
		return f'{field_name}.__str__()'


def create_pod_printer(descriptor, name=None):
	display_type = descriptor.display_type
	if DisplayType.INTEGER == display_type:
		PrinterType = IntPrinter
	elif DisplayType.BYTE_ARRAY == display_type:
		PrinterType = ArrayPrinter
	else:
		PrinterType = TypedArrayPrinter

	return PrinterType(descriptor, name)
