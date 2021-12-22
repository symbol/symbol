from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import indent
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
        method_descriptor.method_name = 'deserialize'
        method_descriptor.arguments = ['payload: bytes']
        method_descriptor.annotations = ['@classmethod']
        return self.generate_method(method_descriptor)

    def generate_create_by_name(self):
        method_descriptor = self.provider.get_create_by_name_descriptor()
        method_descriptor.method_name = 'create_by_name'
        method_descriptor.arguments = [
            'entity_name: str'
        ]
        method_descriptor.annotations = ['@classmethod']
        return self.generate_method(method_descriptor)

    def generate_methods(self):
        methods = []
        methods.append(self.generate_deserializer())
        methods.append(self.generate_create_by_name())
        return methods


class FactoryFormatter(AbstractTypeFormatter):
    def __init__(self, abstract_impl_map, abstract_type_instance):
        super().__init__()

        # array or int
        self.abstract = abstract_type_instance
        self.printer = BuiltinPrinter(abstract_type_instance, 'parent')
        self.factory_descriptor = abstract_impl_map.get(abstract_type_instance.typename)

    def get_ctor_descriptor(self):
        return None

    @property
    def typename(self):
        return self.abstract.typename + 'Factory'

    def create_discriminator(self, name):
        field_names = self.factory_descriptor['discriminator_values']
        values = ', '.join(map(lambda value: f'{name}.{value}', field_names))
        return f'({values}): {name}'

    def get_deserialize_descriptor(self):
        body = 'buffer_ = bytes(payload)\n'
        body += f'{self.printer.name} = {self.printer.load()}\n'

        body += 'mapping = {\n'
        names = [f'{concrete.typename}' for concrete in self.factory_descriptor['children']]
        body += indent(
            ',\n'.join(map(self.create_discriminator, names))
        )
        body += '}\n'

        discriminators = self.factory_descriptor['discriminator_names']
        values = ', '.join(map(lambda discriminator: f'{self.printer.name}.{discriminator}', discriminators))
        body += f'discriminator = ({values})\n'
        body += 'factory_class = mapping[discriminator]\n'
        body += 'return factory_class.deserialize(buffer_)'

        return MethodDescriptor(body=body, result=self.abstract.typename)

    def get_create_by_name_descriptor(self):
        body = ''
        body += 'mapping = {\n'
        body += indent(
            ',\n'.join(
                map(
                    lambda name: f'"{skip_embedded(name.underlined_name)}": {name.typename}',
                    self.factory_descriptor['children'],
                )
            )
        )
        body += '}\n'

        body += f'''
if entity_name not in mapping:
    raise ValueError('unknown {self.printer.get_type()} type')

return mapping[entity_name]()
'''
        return MethodDescriptor(body=body, result=self.abstract.typename)

    def get_serialize_descriptor(self):
        raise RuntimeError('not required')

    def get_size_descriptor(self):
        raise RuntimeError('not required')

    def get_getter_descriptors(self):
        return []

    def get_str_descriptor(self):
        raise RuntimeError('not required')
