from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import IntPrinter


class EnumTypeFormatter(AbstractTypeFormatter):
    def __init__(self, type_instance):
        super().__init__()

        self.enum_type = type_instance
        self.base_type = 'Flag' if 'Flags' in type_instance.typename else 'Enum'

        self.int_printer = IntPrinter(self.enum_type)

    @property
    def typename(self):
        return self.enum_type.typename

    def get_base_class(self):
        return f'({self.base_type})'

    def get_fields(self):
        return list(
            map(
                lambda e: f'{e["name"]} = {e["value"]}\n',
                self.enum_type.values,
            )
        )

    @staticmethod
    def get_ctor_descriptor():
        return None

    def get_deserialize_descriptor(self):
        body = 'buffer_ = memoryview(payload)\n'
        body += f'return {self.typename}({self.int_printer.load()})'
        return MethodDescriptor(body=body)

    def get_serialize_descriptor(self):
        body = 'buffer_ = bytes()\n'
        body += f'buffer_ += {self.int_printer.store("self.value")}\n'
        body += 'return buffer_'
        return MethodDescriptor(body=body)

    def get_size_descriptor(self):
        body = f'return {self.enum_type.size}\n'
        return MethodDescriptor(body=body)

    @staticmethod
    def get_getter_descriptors():
        return []

    @staticmethod
    def get_setter_descriptors():
        return []

    @staticmethod
    def get_str_descriptor():
        return None
