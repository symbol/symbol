from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import IntPrinter


class EnumTypeFormatter(AbstractTypeFormatter):
	def __init__(self, ast_model):
		super().__init__()

		self.enum_type = ast_model
		self.base_type = 'Flag' if self.enum_type.is_bitwise else 'Enum'

		self.int_printer = IntPrinter(self.enum_type)

	@property
	def typename(self):
		return self.enum_type.name

	def get_base_class(self):
		return f'({self.base_type})'

	def get_fields(self):
		return list(
			map(
				lambda e: f'{e.name} = {e.value}\n',
				self.enum_type.values,
			)
		)

	def get_ctor_descriptor(self):
		return None

	def get_deserialize_descriptor(self):
		body = 'buffer = memoryview(payload)\n'
		body += f'return {self.typename}({self.int_printer.load()})'
		return MethodDescriptor(body=body)

	def get_serialize_descriptor(self):
		body = 'buffer = bytes()\n'
		body += f'buffer += {self.int_printer.store("self.value")}\n'
		body += 'return buffer'
		return MethodDescriptor(body=body)

	def get_size_descriptor(self):
		body = f'return {self.enum_type.size}\n'
		return MethodDescriptor(body=body)
