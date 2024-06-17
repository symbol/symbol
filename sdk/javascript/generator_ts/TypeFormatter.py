from generator.format import indent
from generator.TypeFormatter import ClassFormatter


class TypeFormatter(ClassFormatter):
	def generate_class(self):
		output = self.generate_class_header()

		methods = self.generate_methods()
		output += '\n'.join(map(indent, methods))

		output += '}\n'  # class_footer
		return output

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
