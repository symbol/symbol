from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import IntPrinter


class EnumTypeFormatter(AbstractTypeFormatter):
	def __init__(self, ast_model):
		super().__init__()

		self.enum_type = ast_model

		self.int_printer = IntPrinter(self.enum_type)

	@property
	def typename(self):
		return self.enum_type.name

	def get_base_class(self):
		return None

	def get_fields(self):
		return list(
			map(
				lambda e: f'static {e.name} = new {self.typename}({e.value})\n',
				self.enum_type.values,
			)
		)

	@staticmethod
	def get_ctor_descriptor():
		arguments = ['value']
		body = 'this.value = value\n'
		return MethodDescriptor(body=body, arguments=arguments)

	def get_deserialize_descriptor(self):
		body = 'let buffer_ = new Uint8Array(payload.buffer, payload.byteOffset)\n'
		body += '// TODO: make sure the value does not go out of range?\n'
		body += '// TODO: don\'t instantiate\n'
		body += f'return new {self.typename}({self.int_printer.load()})'
		return MethodDescriptor(body=body)

	def get_serialize_descriptor(self):
		return MethodDescriptor(body=f'return {self.int_printer.store("this.value")}')

	def get_size_descriptor(self):
		body = f'return {self.enum_type.size}\n'
		return MethodDescriptor(body=body)

	def get_getter_descriptors(self):
		if not self.enum_type.is_bitwise:
			return []

		body = f'return 0 !== (self.value & flag);\n'
		return [
			MethodDescriptor(method_name='has', arguments=['flag'], body=body)
		]

		body = f'return 0 !== (this.value & flag);\n'
		return [
			MethodDescriptor(method_name='has', arguments=['flag'], body=body)
		]

	@staticmethod
	def get_setter_descriptors():
		return []

	@staticmethod
	def get_str_descriptor():
		return None
