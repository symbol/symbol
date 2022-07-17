from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import wrap_lines
from .printers import IntPrinter


class EnumTypeFormatter(AbstractTypeFormatter):
	def __init__(self, ast_model):
		super().__init__()

		self.enum_type = ast_model

		self.int_printer = IntPrinter(self.enum_type)

	@property
	def typename(self):
		return self.enum_type.name

	def get_fields(self):
		return list(
			map(
				lambda e: f'static {e.name} = new {self.typename}({e.value});\n',
				self.enum_type.values,
			)
		)

	def get_ctor_descriptor(self):
		arguments = ['value']
		body = 'this.value = value;\n'
		return MethodDescriptor(body=body, arguments=arguments)

	def _get_deserialize_descriptor(self, is_aligned):
		body = 'const byteArray = payload;\n'
		if self.enum_type.is_bitwise:
			body += f'return new {self.typename}({self.int_printer.load("byteArray", is_aligned)});'
		else:
			body += f'return this.fromValue({self.int_printer.load("byteArray", is_aligned)});'
		return MethodDescriptor(body=body)

	def get_deserialize_descriptor(self):
		return self._get_deserialize_descriptor(is_aligned=False)

	def get_deserialize_aligned_descriptor(self):
		return self._get_deserialize_descriptor(is_aligned=True)

	def get_serialize_descriptor(self):
		return MethodDescriptor(body=f'return {self.int_printer.store("this.value")};')

	def get_size_descriptor(self):
		body = f'return {self.enum_type.size};\n'
		return MethodDescriptor(body=body)

	def generate_key_value_arrays(self):
		values = list(map(lambda e: str(e.value), self.enum_type.values))
		keys = list(map(lambda e: f'\'{e.name}\'', self.enum_type.values))

		result = wrap_lines(values, 'const values = [', '];\n', 4 * 3)
		result += wrap_lines(keys, 'const keys = [', '];\n', 4 * 3)
		return result

	def get_value_to_key_descriptor(self):
		body = self.generate_key_value_arrays()
		body += '''
const index = values.indexOf(value);
if (-1 === index)
	throw RangeError(`invalid enum value ${value}`);

return keys[index];
'''
		return MethodDescriptor(method_name='static valueToKey', body=body, arguments=['value'])

	def get_map_descriptor(self):
		return MethodDescriptor(method_name='static fromValue', body=f'return {self.typename}[this.valueToKey(value)];', arguments=['value'])

	def get_getter_setter_descriptors(self):
		methods = []

		if self.enum_type.is_bitwise:
			body = 'return 0 !== (this.value & flag);\n'
			methods.append(MethodDescriptor(method_name='has', arguments=['flag'], body=body))
		else:
			methods.append(self.get_value_to_key_descriptor())
			methods.append(self.get_map_descriptor())

		return methods

	def get_str_descriptor(self):
		if not self.enum_type.is_bitwise:
			body = f'return `{self.typename}.${{{self.typename}.valueToKey(this.value)}}`;'
			return MethodDescriptor(body=body)

		body = self.generate_key_value_arrays()
		body += f'''
if (0 === this.value) {{
	const index = values.indexOf(this.value);
	return `{self.typename}.${{keys[index]}}`;
}}

const positions = values.map(flag => (this.value & flag)).filter(n => n).map(n => values.indexOf(n));
return positions.map(n => `{self.typename}.${{keys[n]}}`).join('|');
'''
		return MethodDescriptor(body=body)
