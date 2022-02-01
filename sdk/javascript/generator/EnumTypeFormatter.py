from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import IntPrinter
from .format import wrap_lines

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
				lambda e: f'static {e.name} = new {self.typename}({e.value});\n',
				self.enum_type.values,
			)
		)

	@staticmethod
	def get_ctor_descriptor():
		arguments = ['value']
		body = 'this.value = value;\n'
		return MethodDescriptor(body=body, arguments=arguments)

	def get_deserialize_descriptor(self):
		body = 'const byteArray = payload;\n'
		if self.enum_type.is_bitwise:
			body += f'return new {self.typename}({self.int_printer.load()});'
		else:
			body += f'return this.fromValue({self.int_printer.load()});'
		return MethodDescriptor(body=body)

	def get_serialize_descriptor(self):
		return MethodDescriptor(body=f'return {self.int_printer.store("this.value")};')

	def get_size_descriptor(self):
		body = f'return {self.enum_type.size};\n'
		return MethodDescriptor(body=body)

	def get_map_descriptor(self):
		values = list(map(lambda e: str(e.value), self.enum_type.values))
		keys = list(map(lambda e: f'\'{e.name}\'', self.enum_type.values))

		body = wrap_lines(values, 'const values = [', '];\n', 4*3)
		body += wrap_lines(keys, 'const keys = [', '];\n', 4*3)
		body += f'''
const index = values.indexOf(value);
if (-1 === index)
	throw RangeError(`invalid enum value ${{value}}`);

return {self.typename}[keys[index]];
'''
		return MethodDescriptor(method_name='static fromValue', body=body, arguments=['value'])

	def get_getter_descriptors(self):
		methods = []

		if self.enum_type.is_bitwise:
			body = 'return 0 !== (this.value & flag);\n'
			methods.append(MethodDescriptor(method_name='has', arguments=['flag'], body=body))
		else:
			methods.append(self.get_map_descriptor())

		return methods

	@staticmethod
	def get_setter_descriptors():
		return []

	@staticmethod
	def get_str_descriptor():
		return None
