from catparser.ast import FixedSizeInteger

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
		getters_setters = self.generate_getters_setters()
		methods.extend(getters_setters)
		methods.append(self.generate_deserializer())
		methods.append(self.generate_create_by_name())
		return methods

	def generate_getters_setters(self):
		return list(map(self.generate_method, self.provider.get_getter_setter_descriptors()))


class FactoryFormatter(AbstractTypeFormatter):
	def __init__(self, factory_map, abstract_model):
		super().__init__()

		# array or int
		self.abstract = abstract_model
		self.printer = BuiltinPrinter(abstract_model, 'parent')
		self.factory_descriptor = factory_map.get(self.abstract.name)

	def get_ctor_descriptor(self):
		raise NotImplementedError('`get_ctor_descriptor` not supported by FactoryFormatter')

	@property
	def typename(self):
		return f'{self.abstract.name}Factory'

	@staticmethod
	def map_to_value(name, value, field_type):
		return f'{name}.{value}' if isinstance(field_type, FixedSizeInteger) else f'{name}.{value}.value'

	def create_discriminator(self, name):
		field_values = self.factory_descriptor.discriminator_values
		field_types = self.factory_descriptor.discriminator_types

		values = ', '.join(map(lambda value_type: self.map_to_value(name, *value_type), zip(field_values, field_types)))
		return f'mapping.set({self.typename}.toKey([{values}]), {name});'

	def get_deserialize_descriptor(self):
		body = 'const view = new BufferView(payload);\n'
		body += f'const {self.printer.name} = {self.printer.load()};\n'

		body += 'const mapping = new Map();\n'

		if self.factory_descriptor:
			names = [f'{concrete.name}' for concrete in self.factory_descriptor.children]
			body += '\n'.join(map(self.create_discriminator, names))

		body += '\n'

		discriminators = [] if not self.factory_descriptor else map(fix_name, self.factory_descriptor.discriminator_names)
		discriminator_types = self.factory_descriptor.discriminator_types

		values = ', '.join(map(lambda value_type: self.map_to_value(self.printer.name, *value_type), zip(discriminators, discriminator_types)))
		body += f'const discriminator = {self.typename}.toKey([{values}]);\n'
		body += 'const factory_class = mapping.get(discriminator);\n'
		body += 'return factory_class.deserialize(view.buffer);'

		return MethodDescriptor(body=body)

	def get_create_by_name_descriptor(self):
		body = ''
		body += 'const mapping = {\n'
		body += indent(
			',\n'.join(
				map(
					lambda child: f'{skip_embedded(underline_name(child.name))}: {child.name}',
					[] if not self.factory_descriptor else self.factory_descriptor.children
				)
			)
		)
		body += '};\n'

		body += f'''
if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
	throw RangeError(`unknown {self.printer.get_type()} type ${{entityName}}`);

return new mapping[entityName]();
'''
		return MethodDescriptor(body=body)

	def get_serialize_descriptor(self):
		raise RuntimeError('not required')

	def get_size_descriptor(self):
		raise RuntimeError('not required')

	def get_getter_setter_descriptors(self):
		# toKey is a helper method that is used to create map keys, that are used inside factory's deserialize method
		methods = []
		body = '''if (1 === values.length)
	return values[0];

// assume each key is at most 32bits
return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
'''
		methods.append(MethodDescriptor(method_name='static toKey', arguments=['values'], body=body))
		return methods
