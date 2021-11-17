from AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from format import indent
from printers import create_pod_printer


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

    def get_base_class(self):
        if self.pod.is_array:
            return '(FixedByteArray)'

        return '(BaseValue)'

    def get_ctor_descriptor(self):
        variable_name = self.printer.name
        arguments = [f'{variable_name}: {self.printer.get_type()} = {self.printer.get_default_value()}']
        body = f'{self.printer.check_argument()}'
        body += f'{self.field_name} = {variable_name}'
        return MethodDescriptor(body=body, arguments=arguments)

    def get_deserialize_descriptor(self):
        body = 'buffer_ = memoryview(payload)\n'
        body += f'return {self.typename}({self.printer.load()})'
        return MethodDescriptor(body=body)

    def get_serialize_descriptor(self):
        body = 'buffer_ = bytes()\n'
        body += f'buffer_ += {self.printer.store(self.field_name)}\n'
        body += 'return buffer_'
        return MethodDescriptor(body=body)

    def get_size_descriptor(self):
        body = f'return {self.pod.get_size()}\n'
        return MethodDescriptor(body=body)

    def get_getter_descriptors(self):
        method_descriptor = MethodDescriptor(
            method_name=self.pod.get_underlined_name(),
            body=f'return {self.field_name}',
            result=self.printer.get_type()
        )
        method_descriptor.annotations = ['@property']

        method_eq_body = f'if isinstance(other, {self.typename}):\n'
        method_eq_body += indent(f'return {self.field_name} == other.{self.printer.name}\n')
        method_eq_body += 'return False'
        return [
            method_descriptor,
            MethodDescriptor(
                method_name='__lt__',
                arguments=[f'other: {self.typename}'],
                body=f'return {self.field_name} < other.{self.printer.name}',
                result='bool'
            ),
            MethodDescriptor(
                method_name='__eq__',
                arguments=[f'other: {self.typename}'],
                body=method_eq_body,
                result='bool'
            ),
        ]

    @staticmethod
    def get_setter_descriptors():
        return []

    def get_str_descriptor(self):
        body = f'return {self.printer.to_string(self.field_name)}'
        return MethodDescriptor(body=body)
