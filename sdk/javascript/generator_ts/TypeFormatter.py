from abc import ABC, abstractmethod

from generator.format import indent


class ClassFormatter(ABC):
	def __init__(self, provider):
		self.provider = provider

	@abstractmethod
	def generate_methods(self):
		raise NotImplementedError('need to override method')

	@staticmethod
	def generate_documentation(documentation_lines):
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
		documentation = self.generate_documentation(self.provider.get_class_documentation().splitlines())

		header = f'export class {self.provider.typename} {{\n'
		return f'{documentation}{header}'

	def generate_class(self):
		output = self.generate_class_header()

		methods = self.generate_methods()
		output += '\n'.join(map(indent, methods))

		output += '}\n'  # class_footer
		return output

	def __str__(self):
		return self.generate_class()


class TypeFormatter(ClassFormatter):
	def generate_ctor(self):
		method_descriptor = self.provider.get_ctor_descriptor()
		method_descriptor.method_name = 'constructor'
		return self.generate_method(method_descriptor)

	def generate_to_map(self):
		method_descriptor = self.provider.get_to_map_descriptor()
		return self.generate_method(method_descriptor)

	def generate_methods(self):
		methods = []
		methods.append(self.generate_ctor())
		methods.append(self.generate_to_map())
		return methods
