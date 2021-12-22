class Printer:
    def __init__(self, descriptor, name):
        self.descriptor = descriptor
        # printer.name is 'fixed' field name
        self.name = name or self.descriptor.underlined_name


class IntPrinter(Printer):
    def __init__(self, descriptor, name=None):
        super().__init__(descriptor, name)
        self.type_hint = None

    @staticmethod
    def get_type():
        return 'int'

    @staticmethod
    def get_default_value():
        return '0'

    def get_size(self):
        return self.descriptor.size

    def load(self):
        data_size = self.get_size()
        return f'int.from_bytes(buffer_[:{data_size}], byteorder="little")'

    def advancement_size(self):
        return self.get_size()

    def store(self, field_name):
        return f'{field_name}.to_bytes({self.get_size()}, byteorder="little", signed=False)'

    @staticmethod
    def assign(value):
        return str(value)

    @staticmethod
    def to_string(field_name):
        return f'"0x{{:X}}".format({field_name})'


class TypedArrayPrinter(Printer):
    def __init__(self, descriptor, name=None):
        super().__init__(descriptor, name)
        self.type_hint = f'array[{self.descriptor.element_type.typename}]'

    def get_type(self):
        return f'List[{self.descriptor.get_type()}]'

    @staticmethod
    def get_default_value():
        return '[]'

    def get_size(self):
        if self.descriptor.is_sized:
            # note: use actual `.size` field
            return f'sum(map(lambda e: ArrayHelpers.align_up(e.size(), {self.descriptor.alignment}), self.{self.name}))'

        return f'sum(map(lambda e: e.size(), self.{self.name}))'

    def load(self):
        if self.descriptor.is_sized:
            # use either type name or if it's an abstract type use a factory instead
            factory_name = self.descriptor.get_type()
            if self.descriptor.element_type and self.descriptor.element_type.is_abstract:
                factory_name = self.descriptor.get_type() + 'Factory'

            data_size = self.descriptor.size
            alignment = self.descriptor.alignment
            return f'ArrayHelpers.read_variable_size_elements(buffer_[:{data_size}], {factory_name}, {alignment})'

        if self.descriptor.is_fill:
            return f'ArrayHelpers.read_array(buffer_, {self.descriptor.get_type()})'

        args = [
            'buffer_',
            self.descriptor.get_type(),
            str(self.descriptor.size),
        ]
        if 'sort_key' in self.descriptor.yaml_descriptor:
            sort_key = self.descriptor.yaml_descriptor['sort_key']
            accessor = f'lambda e: e.{sort_key}'
            args.append(accessor)

        args_str = ', '.join(args)
        return f'ArrayHelpers.read_array_count({args_str})'

    def advancement_size(self):
        if self.descriptor.is_sized:
            return str(self.descriptor.size)

        return f'sum(map(lambda e: e.size(), {self.name}))'

    def store(self, field_name):
        if self.descriptor.is_sized:
            return f'ArrayHelpers.write_variable_size_elements({field_name}, {self.descriptor.alignment})'

        if self.descriptor.is_fill:
            return f'ArrayHelpers.write_array({field_name})'

        args = [field_name]
        size = self.descriptor.size
        if not isinstance(size, str):
            args.append(str(size))

        if 'sort_key' in self.descriptor.yaml_descriptor:
            sort_key = self.descriptor.yaml_descriptor['sort_key']
            accessor = f'lambda e: e.{sort_key}'
            args.append(accessor)

        args_str = ', '.join(args)
        if isinstance(size, str):
            return f'ArrayHelpers.write_array({args_str})'

        return f'ArrayHelpers.write_array_count({args_str})'

    @staticmethod
    def to_string(field_name):
        return f'list(map(str, {field_name}))'


class ArrayPrinter(Printer):
    def __init__(self, descriptor, name=None):
        super().__init__(descriptor, name)
        self.type_hint = 'bytes_array'

    @staticmethod
    def get_type():
        return 'bytes'

    def get_default_value(self):
        size = self.descriptor.size
        if isinstance(size, str):
            return 'bytes()'

        return f'bytes({self.get_size()})'

    def get_size(self):
        size = self.descriptor.size
        if isinstance(size, str):
            return f'len(self._{self.name})'

        return size

    def load(self):
        return f'ArrayHelpers.get_bytes(buffer_, {self.advancement_size()})'

    def advancement_size(self):
        # like get_size() but without self prefix, as this refers to local method field
        size = self.descriptor.size
        if isinstance(size, str):
            return str(size)

        return size

    @staticmethod
    def store(field_name):
        return field_name

    @staticmethod
    def to_string(field_name):
        return f'hexlify({field_name}).decode("utf8")'


class BuiltinPrinter(Printer):
    def __init__(self, descriptor, name=None):
        super().__init__(descriptor, name)
        if self.descriptor.is_int:
            self.type_hint = 'pod:'
        elif self.descriptor.is_enum:
            self.type_hint = 'enum:'
        elif self.descriptor.is_array:
            self.type_hint = 'pod:'
        else:
            self.type_hint = 'struct:'

        self.type_hint += self.descriptor.typename

    def get_type(self):
        return self.descriptor.typename

    def get_default_value(self):
        if 'enum' == self.descriptor.base_typename:
            first_enum_value_name = self.descriptor.values[0]['name']
            return f'{self.get_type()}.{first_enum_value_name}'

        return f'{self.get_type()}()'

    def get_size(self):
        return f'self.{self.name}.size()'

    def load(self, buffer_name='buffer_'):
        if self.descriptor.is_struct and self.descriptor.is_abstract:
            # HACK: factories use this printers as well, ignore them
            if 'parent' != self.name:
                factory_name = self.get_type() + 'Factory'
                return f'{factory_name}.deserialize({buffer_name})'

        return f'{self.get_type()}.deserialize({buffer_name})'

    def advancement_size(self):
        return f'{self.name}.size()'

    @staticmethod
    def store(field_name):
        return f'{field_name}.serialize()'

    def assign(self, value):
        return f'{self.get_type()}.{value}'

    @staticmethod
    def to_string(field_name):
        return f'{field_name}.__str__()'


def create_pod_printer(descriptor, name=None):
    size = descriptor.size

    if descriptor.yaml_descriptor['type'] == 'byte':
        PrinterType = ArrayPrinter if isinstance(size, str) or size > 8 else IntPrinter
    else:
        PrinterType = TypedArrayPrinter

    return PrinterType(descriptor, name)
