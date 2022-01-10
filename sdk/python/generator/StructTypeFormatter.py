from itertools import filterfalse

from .AbstractTypeFormatter import AbstractTypeFormatter, MethodDescriptor
from .format import indent
from .type_objects import StructObject


def is_reserved(field):
    disposition = field.yaml_descriptor.get('disposition', None)
    return 'reserved' == disposition


def is_bound_size(field):
    return field.is_bound()


def is_const(field):
    return field.is_const()


def create_temporary_buffer_name(name):
    return f'{name}_condition'


def indent_if_conditional(condition, statements):
    return statements if not condition else condition + indent(statements)


def filter_size_if_first(fields_iter):
    first_field = next(fields_iter, None)
    if 'size' == first_field.original_field_name:
        pass
    else:
        yield first_field

    for field in fields_iter:
        yield field


class StructFormatter(AbstractTypeFormatter):
    # pylint: disable=too-many-public-methods

    def __init__(self, type_instance: StructObject):
        super().__init__()

        self.struct = type_instance

    def non_const_fields(self):
        return filterfalse(is_const, self.struct.layout)

    def const_fields(self):
        return filter(is_const, self.struct.layout)

    def non_reserved_fields(self):
        return filter_size_if_first(filterfalse(is_bound_size, filterfalse(is_reserved, self.non_const_fields())))

    def reserved_fields(self):
        return filter(is_reserved, self.non_const_fields())

    @property
    def typename(self):
        return self.struct.typename

    @staticmethod
    def field_name(field, object_name='self'):
        return f'{object_name}._{field.printer.name}'

    @staticmethod
    def generate_class_field(field):
        default_value = field.printer.assign(field.yaml_descriptor['value'])
        return f'{field.original_field_name}: {field.printer.get_type()} = {default_value}'

    def generate_type_hints(self):
        body = 'TYPE_HINTS = {\n'
        hints = []
        for field in self.non_reserved_fields():
            if not field.printer.type_hint:
                continue
            hints.append(f'"{field.printer.name}": "{field.printer.type_hint}"')

        body += indent(',\n'.join(hints))
        body += '}\n'
        return body

    def get_fields(self):
        return list(map(self.generate_class_field, self.const_fields())) + [self.generate_type_hints()]

    def get_paired_const_field(self, field):
        for const_field in self.const_fields():
            if const_field.original_field_name.lower().endswith(field.original_field_name):
                return const_field
        return None

    def get_ctor_descriptor(self):
        arguments = []

        body = ''
        for field in self.non_reserved_fields():
            const_field = self.get_paired_const_field(field)
            field_name = self.field_name(field)
            if const_field:
                body += f'{field_name} = {self.typename}.{const_field.original_field_name}\n'
            else:
                body += f'{field_name} = {field.printer.get_default_value()}\n'

        body += '\n'.join(
            map(
                lambda field: f'{self.field_name(field)} = {field.yaml_descriptor["value"]}  # reserved field',
                self.reserved_fields()
            )
        )

        if not body:
            return None

        return MethodDescriptor(body=body, arguments=arguments)

    def generate_condition(self, field, prefix_field=False):
        if not field.is_conditional():
            return ''

        condition_field_name = field.yaml_descriptor['condition']

        # find condition field type
        condition_field = next(f for f in self.non_const_fields() if condition_field_name == f.original_field_name)
        condition_to_operator_map = {
            'equals': '==',
            'not equals': '!=',
            'not in': 'not in',
            'in': 'in',
        }
        condition_operator = condition_to_operator_map[field.yaml_descriptor['condition_operation']]

        value = f'{field.yaml_descriptor["condition_value"]}'
        yoda_value = value if condition_field.get_type().is_int else f'{condition_field.get_type().typename}.{value}'
        field_prefix = 'self.' if prefix_field else ''

        # HACK: instead of handling dumb magic value in namespace parent_name, generate slightly simpler condition
        # note: is_array is too wide as it also covers builtin arrays
        if prefix_field and field.yaml_descriptor.get('disposition', '').startswith('array'):
            return f'if {field_prefix}{field.original_field_name}:\n'

        return f'if {yoda_value} {condition_operator} {field_prefix}{condition_field_name}:\n'

    def generate_deserialize_field(self, field, arg_buffer_name=None):
        condition = self.generate_condition(field)

        buffer_name = arg_buffer_name or 'buffer_'

        # half-hack: limit buffer to amount specified in size field
        buffer_load_name = buffer_name
        if field.size_fields:
            assert len(field.size_fields) == 1, f'unexpected number of size_fields associated with {field.original_field_name}'
            buffer_load_name = f'buffer_[:{field.size_fields[0].original_field_name}]'

        use_custom_buffer_name = arg_buffer_name or field.size_fields
        load = field.printer.load(buffer_load_name) if use_custom_buffer_name else field.printer.load()
        deserialize = f'{field.printer.name} = {load}'
        adjust = f'{buffer_name} = {buffer_name}[{field.printer.advancement_size()}:]'

        additional_statements = ''
        if is_reserved(field):
            assert_message = f'f"Invalid value of reserved field ({{{field.printer.name}}})"'
            additional_statements = f'assert {field.printer.name} == {field.yaml_descriptor["value"]}, {assert_message}\n'

        if self.struct.dynamic_size == field.printer.name:
            additional_statements += f'{buffer_name} = {buffer_name}[:size_ - {field.printer.advancement_size()}]\n'
            additional_statements += 'del size_\n'

        if field.is_bound and field.get_type().sizeof_value:
            additional_statements += '# marking sizeof field\n'

        deserialize_field = deserialize + '\n' + adjust + '\n' + additional_statements

        if condition:
            condition = f'{field.printer.name} = None\n' + condition

        return indent_if_conditional(condition, deserialize_field)

    @staticmethod
    def generate_deserialize_to_temporary_buffer(field):
        comment = '# deserialize to temporary buffer for further processing'
        deserialize = f'{field.printer.name}_temporary = {field.printer.load()}'
        temporary_buffer = create_temporary_buffer_name(field.yaml_descriptor['condition'])
        temporary = f'{temporary_buffer} = buffer_[:{field.printer.name}_temporary.size()]'
        adjust = f'buffer_ = buffer_[{field.printer.name}_temporary.size():]'
        return comment + '\n' + deserialize + '\n' + temporary + '\n' + adjust + '\n\n'

    def get_deserialize_descriptor(self):
        body = 'buffer_ = memoryview(payload)\n'

        # special treatment for condition-guarded fields,
        # where condition is behind the fields...
        processed_fields = set()
        queued_fields = {}
        for field in self.non_const_fields():
            if field.is_conditional():
                condition_field_name = field.yaml_descriptor['condition']
                if condition_field_name not in processed_fields:
                    if condition_field_name not in queued_fields:
                        queued_fields[condition_field_name] = []

                        # assume same size and generate single dummy access
                        body += self.generate_deserialize_to_temporary_buffer(field)

                    # queue field for re-reading it from temporary buffer
                    queued_fields[condition_field_name].append({'field': field})
                    continue

            deserialized_field = self.generate_deserialize_field(field)
            body += deserialized_field
            processed_fields.add(field.original_field_name)

            # if conditional field has been processed, process queued fields
            for conditioned in queued_fields.get(field.original_field_name, []):
                body += self.generate_deserialize_field(
                    conditioned['field'],
                    create_temporary_buffer_name(field.original_field_name),
                )

        # create call to ctor
        body += '\n'
        body += f'instance = {self.typename}()\n'

        for field in self.non_reserved_fields():
            field_name = self.field_name(field, 'instance')
            body += f'{field_name} = {field.printer.name}\n'

        body += 'return instance'
        return MethodDescriptor(body=body)

    def generate_serialize_field(self, field):
        condition = self.generate_condition(field, True)

        field_value = ''
        field_comment = ''

        # bound fields are the size / count / sizeof fields that are bound to either object or array
        if field.is_bound():
            bound_field = field.bound_field
            bound_field_name = self.field_name(bound_field)
            field_comment = f'  # {field.original_field_name}'

            if bound_field.get_type().is_array:
                if field.original_field_name.endswith('_count') or not bound_field.get_type().is_sized:
                    field_value = f'len({bound_field_name})'

                    bound_condition = self.generate_condition(bound_field, True)
                    if condition and bound_condition:
                        raise RuntimeError('do not know yet how to generate both conditions')

                    # HACK: create inline if condition (for NEM namespace purposes)
                    if bound_condition:
                        condition_value = bound_field.yaml_descriptor['condition_value']
                        field_value = f'({field_value} if {bound_field_name} is not None else {condition_value})'
                else:
                    field_value = field.bound_field.printer.get_size()
            elif field.get_type().sizeof_value:
                field_value = field.bound_field.printer.get_size()
        else:
            field_value = self.field_name(field)

        serialize_field = field.printer.store(field_value) + field_comment

        return indent_if_conditional(condition, f'buffer_ += {serialize_field}\n')

    def get_serialize_descriptor(self):
        body = 'buffer_ = bytes()\n'

        # if first field is size replace serializer with custom one (to access builder .size() instead)
        fields_iter = self.non_const_fields()
        first_field = next(fields_iter)
        if self.struct.dynamic_size == first_field.printer.name:
            body += f'buffer_ += self.size().to_bytes({first_field.get_type().size}, byteorder="little", signed=False)\n'
        else:
            body += self.generate_serialize_field(first_field)

        for field in fields_iter:
            body += self.generate_serialize_field(field)

        body += 'return buffer_'
        return MethodDescriptor(body=body)

    def generate_size_field(self, field):
        condition = self.generate_condition(field, True)
        size_field = field.printer.get_size()

        return indent_if_conditional(condition, f'size += {size_field}\n')

    def get_size_descriptor(self):
        body = 'size = 0\n'
        body += ''.join(map(self.generate_size_field, self.non_const_fields()))
        body += 'return size'
        return MethodDescriptor(body=body)

    def create_getter_descriptor(self, field):
        method_descriptor = MethodDescriptor(
            method_name=field.printer.name,
            body=f'return {self.field_name(field)}',
            result=field.printer.get_type(),
        )
        method_descriptor.annotations = ['@property']
        return method_descriptor

    def get_getter_descriptors(self):
        return list(map(self.create_getter_descriptor, self.non_reserved_fields()))

    def create_setter_descriptor(self, field):
        method_descriptor = MethodDescriptor(
            method_name=field.printer.name,
            arguments=[f'value: {field.printer.get_type()}'],
            body=f'{self.field_name(field)} = value',
        )
        method_descriptor.annotations = [f'@{field.printer.name}.setter']
        return method_descriptor

    def get_setter_descriptors(self):
        return list(map(self.create_setter_descriptor, self.non_reserved_fields()))

    def generate_str_field(self, field):
        field_to_string = field.printer.to_string(self.field_name(field))
        return f'"{field.printer.name}: {{}}, ".format({field_to_string})'

    def get_str_descriptor(self):
        body = 'result = "("\n'
        body += ''.join(
            map(
                'result += {}\n'.format,  # pylint: disable=consider-using-f-string
                map(self.generate_str_field, self.non_reserved_fields()),
            )
        )
        body += 'result += ")"\n'
        body += 'return result'
        return MethodDescriptor(body=body)
