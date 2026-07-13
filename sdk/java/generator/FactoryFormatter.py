"""Emits a Java ``XFactory`` class that dispatches abstract-struct deserialization."""

from catparser.ast import FixedSizeInteger

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .name_formatting import capitalize, underline_name
from .printers import BuiltinPrinter


def _skip_embedded(name):
	if not name.startswith('embedded_'):
		return name
	return name[len('embedded_'):]


class FactoryFormatter(AbstractTypeFormatter):
	def __init__(self, factory_map, abstract_model):
		super().__init__()
		self.abstract = abstract_model
		self.printer = BuiltinPrinter(abstract_model, 'parent')
		self.factory_descriptor = factory_map.get(self.abstract.name)

	@property
	def typename(self):
		return f'{self.abstract.name}Factory'

	def get_ctor_descriptor(self):
		# Factory has only static methods; emit a private no-op ctor to prevent instantiation
		descriptor = MethodDescriptor(body='// no instances', arguments=[])
		descriptor.visibility = 'private'
		return descriptor

	def get_size_descriptor(self):
		return None

	def get_serialize_descriptor(self):
		return None

	@staticmethod
	def _value_expr(expr, field_type):
		# enum discriminators expose their numeric key via getValue(); plain integers are used as-is
		return expr if isinstance(field_type, FixedSizeInteger) else f'{expr}.getValue()'

	@staticmethod
	def _map_to_value(name, value, field_type):
		# Discriminator literal: enum -> enum_constant.getValue(), int -> raw literal
		return FactoryFormatter._value_expr(f'{name}.{value}', field_type)

	def _create_discriminator(self, name):
		field_values = self.factory_descriptor.discriminator_values
		field_types = self.factory_descriptor.discriminator_types
		values = ', '.join(self._map_to_value(name, *vt) for vt in zip(field_values, field_types))
		return f'mapping.put(toKey({values}), {name}::deserialize);'

	def get_deserialize_descriptor(self):
		body = (
			f'final org.symbol.sdk.utils.BufferView discriminatorView = view.snapshot();\n'
			f'final {self.abstract.name} parent = new {self.abstract.name}();\n'
			f'{self.abstract.name}.deserializeInto(discriminatorView, parent);\n\n'
			f'final java.util.Map<Long, java.util.function.Function<org.symbol.sdk.utils.BufferView, ? extends {self.abstract.name}>>'
			f' mapping = new java.util.HashMap<>();\n'
		)

		if self.factory_descriptor:
			names = [child.name for child in self.factory_descriptor.children]
			body += '\n'.join(self._create_discriminator(name) for name in names) + '\n\n'
		else:
			body += '\n'

		if self.factory_descriptor:
			discriminators = list(self.factory_descriptor.discriminator_names)
			discriminator_types = self.factory_descriptor.discriminator_types
		else:
			discriminators, discriminator_types = [], []

		# the emitted toKey overloads (see _to_key_descriptors) pack at most two discriminators into a long.
		if not 1 <= len(discriminators) <= 2:
			raise RuntimeError(f'{self.abstract.name} has {len(discriminators)} discriminators but toKey supports only 1 or 2')
		# read each discriminator off the deserialized parent via its getter
		value_exprs = []
		for name, field_type in zip(discriminators, discriminator_types):
			getter = f'parent.get{capitalize(name)}()'
			value_exprs.append(self._value_expr(getter, field_type))

		values = ', '.join(value_exprs)
		body += (
			f'final long discriminator = toKey({values});\n'
			f'final java.util.function.Function<org.symbol.sdk.utils.BufferView, ? extends {self.abstract.name}> factoryFn'
			f' = mapping.get(discriminator);\n'
			f'if (factoryFn == null)\n'
			f'\tthrow new IllegalArgumentException("unknown {self.abstract.name} discriminator " + discriminator);\n'
			f'return factoryFn.apply(view);'
		)
		descriptor = MethodDescriptor(body=body, arguments=['org.symbol.sdk.utils.BufferView view'])
		descriptor.result = self.abstract.name
		descriptor.is_static = True
		return descriptor

	def get_create_by_name_descriptor(self):
		body = f'final java.util.Map<String, java.util.function.Supplier<{self.abstract.name}>> mapping = new java.util.HashMap<>();\n'
		if self.factory_descriptor:
			for child in self.factory_descriptor.children:
				key = _skip_embedded(underline_name(child.name))
				body += f'mapping.put("{key}", {child.name}::new);\n'

		body += (
			f'final java.util.function.Supplier<{self.abstract.name}> supplier = mapping.get(entityName);\n'
			f'if (supplier == null)\n'
			f'\tthrow new IllegalArgumentException("unknown {self.abstract.name} type " + entityName);\n'
			f'return supplier.get();'
		)
		descriptor = MethodDescriptor(method_name='createByName', body=body, arguments=['String entityName'])
		descriptor.result = self.abstract.name
		descriptor.is_static = True
		return descriptor

	def _to_key_descriptors(self):  # pylint: disable=no-self-use
		single = MethodDescriptor(method_name='toKey', body='return value;', arguments=['long value'])
		single.result = 'long'
		single.is_static = True
		single.visibility = 'private'

		pair = MethodDescriptor(
			method_name='toKey',
			body='return (a << 32) | (b & 0xFFFFFFFFL);',
			arguments=['long a', 'long b'])
		pair.result = 'long'
		pair.is_static = True
		pair.visibility = 'private'
		return [single, pair]

	def get_getter_setter_descriptors(self):
		# Expose the createByName + toKey static helpers via the standard descriptor pipeline.
		return [self.get_create_by_name_descriptor(), *self._to_key_descriptors()]
