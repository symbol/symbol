from abc import ABC, abstractmethod


class MethodDescriptor:
    def __init__(self, method_name=None, arguments=None, body='pass', result=''):
        self.method_name = method_name
        self.arguments = arguments or []
        self.body = body
        self.result = result
        self.annotations = []


class AbstractTypeFormatter(ABC):
    @property
    @abstractmethod
    def typename(self):
        raise NotImplementedError('need to override method')

    @staticmethod
    def get_base_class():
        return ''

    @abstractmethod
    def get_ctor_descriptor(self) -> MethodDescriptor:
        pass

    @abstractmethod
    def get_deserialize_descriptor(self) -> MethodDescriptor:
        pass

    @abstractmethod
    def get_serialize_descriptor(self) -> MethodDescriptor:
        pass

    @abstractmethod
    def get_size_descriptor(self) -> MethodDescriptor:
        pass

    @abstractmethod
    def get_getter_descriptors(self) -> MethodDescriptor:
        pass

    @abstractmethod
    def get_str_descriptor(self) -> MethodDescriptor:
        pass

    @staticmethod
    def get_fields():
        return []
