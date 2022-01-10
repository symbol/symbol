from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .printers import create_pod_printer


class PodTypeFormatter(AbstractTypeFormatter):
    def __init__(self, type_instance):
        super().__init__()

        # array or int
        self.pod = type_instance
        self.printer = create_pod_printer(self.pod)

    @property
    def typename(self):
        return self.pod.typename

    @property
    def field_name(self):
        return f'self._{self.printer.name}'

    def get_fields(self):
        return [f'SIZE = {self.pod.size}']

    def get_base_class(self):
        return '(ByteArray)' if self.pod.is_array else '(BaseValue)'

    def get_ctor_descriptor(self):
        variable_name = self.printer.name
        body = f'super().__init__(self.SIZE, {variable_name}, {self.typename})'
        if self.pod.is_array:
            arguments = [f'{variable_name}: StrBytes = {self.printer.get_default_value()}']
        else:
            arguments = [f'{variable_name}: {self.printer.get_type()} = {self.printer.get_default_value()}']

        return MethodDescriptor(body=body, arguments=arguments)

    def get_deserialize_descriptor(self):
        body = 'buffer_ = memoryview(payload)\n'
        body += f'return {self.typename}({self.printer.load()})'
        return MethodDescriptor(body=body)

    def get_serialize_descriptor(self):
        if self.pod.is_array:
            return MethodDescriptor(body='return self.bytes')

        return MethodDescriptor(body=f'return {self.printer.store("self.value")}')

    def get_size_descriptor(self):
        body = f'return {self.pod.size}\n'
        return MethodDescriptor(body=body)

    def get_getter_descriptors(self):
        return []

    @staticmethod
    def get_setter_descriptors():
        return []

    def get_str_descriptor(self):
        return None
