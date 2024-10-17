from abc import ABC, abstractmethod
from .format import indent


class ClassFormatter(ABC):
    def __init__(self, provider):
        self.provider = provider

    @abstractmethod
    def generate_methods(self):
        raise NotImplementedError('need to override method')

    # ドキュメントコメントを生成する
    @staticmethod
    def generate_documentation(documentation_lines):
        if not documentation_lines:
            return ''
        documentation = '/*\n'
        documentation += '\n'.join(documentation_lines)
        documentation += '\n */\n'
        return documentation

    # 関数の生成
    @staticmethod
    def generate_method(method_descriptor):
        virtual = 'virtual' if method_descriptor.is_virtual else ''
        arguments = ', '.join(method_descriptor.arguments)
        if len(arguments) > 100:
            arguments = '\n\t' + ',\n\t'.join(method_descriptor.arguments) + '\n'

        is_constructor = method_descriptor.method_name == 'constructor' # constructor,voidは戻り値がない
        method_result = '' if is_constructor else method_descriptor.result
        body = indent(method_descriptor.body)

        if virtual!='':
            return f'{virtual} {method_result} {method_descriptor.method_name}({arguments}) {{\n{body}}}'
        else:
            return f'{method_result} {method_descriptor.method_name}({arguments}) {{\n{body}}}'

	# クラスのヘッダーを生成する
    def generate_class_header(self):
        base_class = self.provider.get_base_class()
        base_class = f' : {base_class}' if base_class else ''
        header = f'class {self.provider.typename}{base_class} {{\n'
        header += 'public:\n'
        return f'{header}'

    def generate_class(self):
        output = self.generate_class_header()

        fields = self.provider.get_fields()
        fields_output = ''
        for field in fields:
            fields_output += indent(field + ';')

        if fields_output:
            output += fields_output + '\n'

        methods = self.generate_methods()
        output += '\n'.join(map(indent, methods))
        output += '\n};'
        return output

    def generate_output(self):
        return self.generate_class()

    def __str__(self):
        return self.generate_output()


def _append_if_not_none(methods, descriptor):
    if not descriptor:
        return
    methods.append(descriptor)


class TypeFormatter(ClassFormatter):
    def generate_ctor(self):
        method_descriptor = self.provider.get_ctor_descriptor()
        if not method_descriptor:
            return None

        method_descriptor.method_name = self.provider.typename  # C++のコンストラクタはクラス名と一致する
        return self.generate_method(method_descriptor)

    def generate_comparer(self):
        method_descriptor = self.provider.get_comparer_descriptor()
        if not method_descriptor:
            return None

        method_descriptor.method_name = 'comparer'
        # method_descriptor.arguments = ['const MyClass& other']  # 比較対象のオブジェクトを引数に追加
        return self.generate_method(method_descriptor)

    def generate_sort(self):
        method_descriptor = self.provider.get_sort_descriptor()
        if not method_descriptor:
            return None

        method_descriptor.method_name = 'sort'
        method_descriptor.arguments = []
        return self.generate_method(method_descriptor)

    def generate_deserializer(self):
        method_descriptor = self.provider.get_deserialize_descriptor()
        method_descriptor.method_name = 'deserialize'
        method_descriptor.result = self.provider.typename
        method_descriptor.arguments = ['const std::vector<uint8_t>& buffer']
        return self.generate_method(method_descriptor)

    def generate_serializer(self):
        method_descriptor = self.provider.get_serialize_descriptor()
        method_descriptor.method_name = 'serialize'
        method_descriptor.result = 'std::vector<uint8_t>'
        return self.generate_method(method_descriptor)

    def generate_size(self):
        method_descriptor = self.provider.get_size_descriptor()
        if not method_descriptor:
            return None

        method_descriptor.method_name = 'size'
        method_descriptor.result = 'size_t'
        return self.generate_method(method_descriptor)

    def generate_getters(self):
        return list(map(self.generate_method, self.provider.get_getter_descriptors()))

    def generate_setters(self):
        return list(map(self.generate_method, self.provider.get_setter_descriptors()))

    def generate_representation(self):
        method_descriptor = self.provider.get_str_descriptor()
        if not method_descriptor:
            return None

        method_descriptor.method_name = 'to_string'
        method_descriptor.result = 'std::string'
        return self.generate_method(method_descriptor)
    
    def generate_json(self):
        method_descriptor = self.provider.get_json_descriptor()
        if not method_descriptor:
            return None
        method_descriptor.method_name = 'to_json'
        method_descriptor.result = 'std::string'
        return self.generate_method(method_descriptor)

    def generate_methods(self):
        methods = []

        _append_if_not_none(methods, self.generate_ctor())
        _append_if_not_none(methods, self.generate_comparer())
        _append_if_not_none(methods, self.generate_sort())

        getters = self.generate_getters()
        methods.extend(getters)

        setters = self.generate_setters()
        methods.extend(setters)

        _append_if_not_none(methods, self.generate_size())

        methods.append(self.generate_deserializer())
        methods.append(self.generate_serializer())

        _append_if_not_none(methods, self.generate_representation())
        _append_if_not_none(methods, self.generate_json())

        return methods

    def __str__(self):
        return self.generate_output()
