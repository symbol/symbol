from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import indent
from .name_formatting import fix_name, underline_name
from .printers import BuiltinPrinter
from .TypeFormatter import ClassFormatter


# hack: skip embedded from names
def skip_embedded(name):
	if not name.startswith('embedded_'):
		return name

	return name[len('embedded_'):]


class FactoryClassFormatter(ClassFormatter):
	def generate_deserializer(self):
		method_descriptor = self.provider.get_deserialize_descriptor()
		method_descriptor.method_name = 'static deserialize'
		method_descriptor.arguments = ['payload']
		return self.generate_method(method_descriptor)

	def generate_create_by_name(self):
		method_descriptor = self.provider.get_create_by_name_descriptor()
		method_descriptor.method_name = 'static createByName'
		method_descriptor.arguments = [
			'entityName'
		]
		return self.generate_method(method_descriptor)

	def generate_methods(self):
		methods = []
		methods.append(self.generate_deserializer())
		methods.append(self.generate_create_by_name())
		return methods


class FactoryFormatter(AbstractTypeFormatter):
	def __init__(self, abstract_impl_map, abstract_model):
		super().__init__()

		# array or int
		self.abstract = abstract_model
		self.printer = BuiltinPrinter(abstract_model, 'parent')
		self.factory_descriptor = abstract_impl_map.get(self.abstract.name)

	def get_ctor_descriptor(self):
		return None

	@property
	def typename(self):
		return f'{self.abstract.name}Factory'

	def create_discriminator(self, name):
		field_names = self.factory_descriptor['discriminator_values']
		values = ', '.join(map(lambda value: f'{name}.{value}.value', field_names))
		return f'[{values}]: {name}'

	def get_deserialize_descriptor(self):
		body = f'const view = new BufferView(payload);\n';
		body += f'const {self.printer.name} = {self.printer.load()};\n'

		body += 'const mapping = {\n'

		if self.factory_descriptor:
			names = [f'{concrete.name}' for concrete in self.factory_descriptor['children']]
			body += indent(
				',\n'.join(map(self.create_discriminator, names))
			)

		body += '};\n'

		discriminators = [] if not self.factory_descriptor else self.factory_descriptor['discriminator_names']
		values = ', '.join(map(lambda discriminator: f'{self.printer.name}.{fix_name(discriminator)}', discriminators))
		body += f'const discriminator = {values};\n'
		body += 'const factory_class = mapping[discriminator.value];\n'
		body += 'return factory_class.deserialize(view.buffer);'

		return MethodDescriptor(body=body)

	def get_create_by_name_descriptor(self):
		body = ''
		body += 'const mapping = {\n'
		body += indent(
			',\n'.join(
				map(
					lambda child: f'"{skip_embedded(underline_name(child.name))}": {child.name}',
					[] if not self.factory_descriptor else self.factory_descriptor['children']
				)
			)
		)
		body += '};\n'

		body += f'''
if (!mapping.hasOwnProperty(entityName))
	throw RangeError('unknown {self.printer.get_type()} type')

return new mapping[entityName]()
'''
		return MethodDescriptor(body=body)

	def get_serialize_descriptor(self):
		raise RuntimeError('not required')

	def get_size_descriptor(self):
		raise RuntimeError('not required')

	def get_getter_descriptors(self):
		return []

	def get_str_descriptor(self):
		raise RuntimeError('not required')
