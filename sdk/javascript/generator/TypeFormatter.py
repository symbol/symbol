from abc import ABC, abstractmethod

from .format import indent


class ClassFormatter(ABC):
	def __init__(self, provider):
		self.provider = provider

	@abstractmethod
	def generate_methods(self):
		raise NotImplementedError('need to override method')

	@staticmethod
	def generate_documentation(documentation_lines):
		if not documentation_lines:
			return ''

		def prepend_star(line):
			return ' *' if not line else f' * {line}'

		documentation = '/**\n'
		documentation += '\n'.join(prepend_star(line) for line in documentation_lines)
		documentation += '\n */\n'
		return documentation

	@staticmethod
	def generate_method(method_descriptor):
		arguments = ', '.join(method_descriptor.arguments)
		if len(arguments) > 100:
			arguments = '\n\t' + ',\n\t'.join(method_descriptor.arguments) + '\n'

		documentation = ClassFormatter.generate_documentation(method_descriptor.documentation)
		body = indent(method_descriptor.body)

		disabled_warnings = ''
		if method_descriptor.disabled_warnings:
			disabled_warnings = f' // eslint-disable-line {" ".join(method_descriptor.disabled_warnings)}'

		return f'{documentation}{method_descriptor.method_name}({arguments}) {{{disabled_warnings}\n{body}}}\n'

	def generate_class_header(self):
		documentation = ''
		if self.provider.get_class_documentation():
			documentation = ClassFormatter.generate_documentation(self.provider.get_class_documentation().splitlines())

		base_class = self.provider.get_base_class()
		base_class = f' extends {base_class}' if base_class else ''
		header = f'export class {self.provider.typename}{base_class} {{\n'
		return f'{documentation}{header}'

	def generate_class(self):
		output = self.generate_class_header()

		# additional newline between fields for js linter
		fields = self.provider.get_fields()
		fields_output = indent('\n'.join(fields))

		if fields_output:
			output += fields_output + '\n'

		methods = self.generate_methods()
		output += '\n'.join(map(indent, methods))

		output += '}\n'  # class_footer
		return output

	def generate_output(self):
		output = self.generate_class()
		return output

	def __str__(self):
		return self.generate_output()


def _append_if_not_none(methods, descriptor):
	if not descriptor:
		return

	methods.append(descriptor)


class TypeFormatter(ClassFormatter):
	def generate_ctor(self):
		method_descriptor = self.provider.get_ctor_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'constructor'
		return self.generate_method(method_descriptor)

	def generate_comparer(self):
		method_descriptor = self.provider.get_comparer_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'comparer'
		method_descriptor.arguments = []
		return self.generate_method(method_descriptor)

	def generate_sort(self):
		method_descriptor = self.provider.get_sort_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'sort'
		method_descriptor.arguments = []
		if not method_descriptor.body:
			method_descriptor.disabled_warnings = ['class-methods-use-this']

		return self.generate_method(method_descriptor)

	def generate_deserializer(self, name, generated_name=None):
		if not hasattr(self.provider, f'get_{name}_descriptor'):
			return None

		descriptor_getter = getattr(self.provider, f'get_{name}_descriptor')
		method_descriptor = descriptor_getter()
		if not method_descriptor:
			return None

		generated_name = generated_name or name
		prefix = '_' if self.provider.is_type_abstract else ''
		method_descriptor.method_name = f'static {prefix}{generated_name}'
		method_descriptor.arguments = ['view', 'instance'] if self.provider.is_type_abstract else ['payload']

		method_descriptor.annotations = []
		return self.generate_method(method_descriptor)

	def generate_serializer(self):
		method_descriptor = self.provider.get_serialize_descriptor()
		method_descriptor.method_name = 'serialize'
		return self.generate_method(method_descriptor)

	def generate_serializer_protected(self):
		method_descriptor = self.provider.get_serialize_protected_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = '_serialize'
		method_descriptor.arguments = ['buffer']
		return self.generate_method(method_descriptor)

	def generate_size(self):
		method_descriptor = self.provider.get_size_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'get size'
		method_descriptor.arguments = []
		method_descriptor.disabled_warnings = ['class-methods-use-this']
		return self.generate_method(method_descriptor)

	def generate_getters_setters(self):
		return list(map(self.generate_method, self.provider.get_getter_setter_descriptors()))

	def generate_representation(self):
		method_descriptor = self.provider.get_str_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'toString'
		return self.generate_method(method_descriptor)

	def generate_json(self):
		method_descriptor = self.provider.get_json_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'toJson'
		return self.generate_method(method_descriptor)

	def generate_methods(self):
		methods = []

		_append_if_not_none(methods, self.generate_ctor())
		_append_if_not_none(methods, self.generate_comparer())
		_append_if_not_none(methods, self.generate_sort())

		methods.extend(self.generate_getters_setters())

		_append_if_not_none(methods, self.generate_size())

		methods.append(self.generate_deserializer('deserialize'))
		_append_if_not_none(methods, self.generate_deserializer('deserialize_aligned', 'deserializeAligned'))

		methods.append(self.generate_serializer())
		_append_if_not_none(methods, self.generate_serializer_protected())

		_append_if_not_none(methods, self.generate_representation())
		_append_if_not_none(methods, self.generate_json())

		return methods

	def __str__(self):
		return self.generate_output()
