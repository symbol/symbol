import re


class BaseObject:
    # pylint: disable=too-many-instance-attributes

    CAMEL_CASE_PATTERN = re.compile(r'(?<!^)(?=[A-Z])')

    def __init__(self, base_typename, yaml_descriptor):
        self.base_typename = base_typename
        self.is_struct = 'struct' == base_typename
        self.is_array = 'array' == base_typename
        self.is_int = 'int' == base_typename
        self.is_enum = 'enum' == base_typename

        self.is_builtin = True
        self.sizeof_value = yaml_descriptor['value'] if 'sizeof' == yaml_descriptor.get('disposition', None) else None

        self.yaml_descriptor = yaml_descriptor
        self.field_name = BaseObject.CAMEL_CASE_PATTERN.sub('_', self.yaml_descriptor['name']).lower()

        self.printer = None
        self.typename = self.yaml_descriptor['name']

    def set_printer(self, printer):
        self.printer = printer

    @property
    def underlined_name(self):
        return self.field_name

    @property
    def size(self):
        return self.yaml_descriptor['size']


class IntObject(BaseObject):
    def __init__(self, yaml_descritor):
        super().__init__('int', yaml_descritor)


class ArrayObject(BaseObject):
    def __init__(self, yaml_descritor):
        super().__init__('array', yaml_descritor)
        self.element_type = None

    @property
    def is_sized(self):
        return 'array sized' == self.yaml_descriptor['disposition']

    @property
    def is_fill(self):
        return 'array fill' == self.yaml_descriptor['disposition']

    @property
    def alignment(self):
        return self.yaml_descriptor['alignment']

    def get_type(self):
        return self.yaml_descriptor['type']


class EnumObject(BaseObject):
    def __init__(self, yaml_descritor):
        super().__init__('enum', yaml_descritor)

    @property
    def underlined_name(self):
        return 'value'

    @property
    def values(self):
        return self.yaml_descriptor['values']


class StructObject(BaseObject):
    def __init__(self, yaml_descritor):
        super().__init__('struct', yaml_descritor)
        self.layout = []
        self.dynamic_size = self.yaml_descriptor.get('size', None)
        self.name_to_index = {}
        self.is_abstract = 'abstract' == self.yaml_descriptor.get('disposition', None)

    def add_field(self, field):
        self.name_to_index[field.original_field_name] = len(self.layout)
        self.layout.append(field)

    def get_field_by_name(self, field_name):
        index = self.name_to_index[field_name]
        return self.layout[index]
