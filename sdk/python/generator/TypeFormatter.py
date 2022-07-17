from abc import ABC, abstractmethod

from .format import indent


class ClassFormatter(ABC):
	def __init__(self, provider):
		self.provider = provider

	@abstractmethod
	def generate_methods(self):
		raise NotImplementedError('need to override method')

	@staticmethod
	def generate_method(method_descriptor):
		all_args = (['cls'] if '@classmethod' in method_descriptor.annotations else ['self']) + method_descriptor.arguments
		arguments = ', '.join(all_args)
		if len(arguments) > 100:
			arguments = '\n    ' + ',\n    '.join(all_args) + '\n'

		annotations = '\n'.join(method_descriptor.annotations + [''])

		is_void = annotations.endswith('setter\n') or method_descriptor.method_name == '__init__'
		method_result = '' if is_void else f' -> {method_descriptor.result}'
		body = indent(method_descriptor.body)
		return f'{annotations}def {method_descriptor.method_name}({arguments}){method_result}:\n{body}'

	def generate_class_header(self):
		header = f'class {self.provider.typename}{self.provider.get_base_class()}:\n'
		comment = ''
		return header + indent(comment)

	def generate_class(self):
		output = self.generate_class_header()

		fields = self.provider.get_fields()
		fields_output = ''
		for field in fields:
			fields_output += indent(field)

		if fields_output:
			output += fields_output + '\n'

		methods = self.generate_methods()
		output += '\n'.join(map(indent, methods))
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

		method_descriptor.method_name = '__init__'
		return self.generate_method(method_descriptor)

	def generate_comparer(self):
		method_descriptor = self.provider.get_comparer_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'comparer'
		method_descriptor.arguments = []
		method_descriptor.result = 'tuple'
		return self.generate_method(method_descriptor)

	def generate_sort(self):
		method_descriptor = self.provider.get_sort_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'sort'
		method_descriptor.arguments = []
		method_descriptor.result = 'None'
		return self.generate_method(method_descriptor)

	def generate_deserializer(self):
		# 'deserialize'
		method_descriptor = self.provider.get_deserialize_descriptor()
		method_descriptor.method_name = 'deserialize'
		method_descriptor.arguments = ['payload: ByteString']
		method_descriptor.result = self.provider.typename
		method_descriptor.annotations = ['@classmethod']
		return self.generate_method(method_descriptor)

	def generate_serializer(self):
		method_descriptor = self.provider.get_serialize_descriptor()
		method_descriptor.method_name = 'serialize'
		method_descriptor.result = 'bytes'
		return self.generate_method(method_descriptor)

	def generate_size(self):
		method_descriptor = self.provider.get_size_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = 'size'
		method_descriptor.arguments = []
		method_descriptor.result = 'int'
		method_descriptor.annotations = ['@property']
		return self.generate_method(method_descriptor)

	def generate_getters(self):
		return list(map(self.generate_method, self.provider.get_getter_descriptors()))

	def generate_setters(self):
		return list(map(self.generate_method, self.provider.get_setter_descriptors()))

	def generate_representation(self):
		method_descriptor = self.provider.get_str_descriptor()
		if not method_descriptor:
			return None

		method_descriptor.method_name = '__str__'
		method_descriptor.result = 'str'
		return self.generate_method(method_descriptor)

	def generate_methods(self):
		methods = []

		_append_if_not_none(methods, self.generate_ctor())
		_append_if_not_none(methods, self.generate_comparer())
		_append_if_not_none(methods, self.generate_sort())

		getters = self.generate_getters()
		methods.extend(getters)
		setters = self.generate_setters()
		methods.extend(setters)

		_append_if_not_none(methods, self.generate_size())

		methods.append(self.generate_deserializer())
		methods.append(self.generate_serializer())

		_append_if_not_none(methods, self.generate_representation())

		return methods

	def __str__(self):
		return self.generate_output()
