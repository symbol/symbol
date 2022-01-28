import functools
import inspect
import operator
from enum import Enum, Flag

from .BaseValue import BaseValue
from .ByteArray import ByteArray
from .TransactionDescriptorProcessor import TransactionDescriptorProcessor


def _name_to_enum_value(mapping, enum_type, enum_value_name):
	if enum_value_name not in mapping:
		raise ValueError(f'unknown value {enum_value_name} for type {enum_type}')

	return mapping[enum_value_name]


def _build_type_hints_map(struct_value):
	type_hints = {}
	for key, hint in struct_value.TYPE_HINTS.items():
		rule_name = None
		if hint.startswith('array['):
			rule_name = hint
		elif hint.startswith('enum:'):
			rule_name = hint[len('enum:'):]
		elif hint.startswith('pod:'):
			rule_name = hint[len('pod:'):]
		elif hint.startswith('struct:'):
			rule_name = hint

		if rule_name:
			type_hints[key] = rule_name

	return type_hints


def _type_converter_factory(module, custom_type_converter, value):
	if custom_type_converter and custom_type_converter(value):
		return custom_type_converter(value)

	if isinstance(value, ByteArray):
		return getattr(module, type(value).__name__)(value.bytes)

	return value


class RuleBasedTransactionFactory:
	"""Rule based transaction factory."""

	def __init__(self, module, type_converter=None, type_rule_overrides=None):
		"""Creates a rule based transaction factory for use with catbuffer generated code."""
		self.module = module
		self.type_converter = lambda value: _type_converter_factory(self.module, type_converter, value)
		self.type_rule_overrides = type_rule_overrides or {}
		self.rules = {}

	def _get_module_class(self, name):
		return getattr(self.module, name)

	def add_pod_parser(self, name, pod_class):
		"""Creates wrapper for SDK POD types."""

		if pod_class in self.type_rule_overrides:
			# if an override has been set for the type, use it
			self.rules[name] = self.type_rule_overrides[pod_class]
			return

		self.rules[name] = lambda value: pod_class(value) if not isinstance(value, pod_class) else value

	def add_flags_parser(self, name):
		"""Creates flag type parser."""
		flags_class = self._get_module_class(name)
		string_to_enum = dict(map(lambda key: (key.name.lower(), key), flags_class))

		def parser(flags):
			if isinstance(flags, str):
				enum_array = list(map(lambda flag_name: _name_to_enum_value(string_to_enum, name, flag_name), flags.split(' ')))
				return functools.reduce(operator.or_, enum_array)

			if isinstance(flags, int):
				return flags_class(flags)

			return flags

		self.rules[name] = parser

	def add_enum_parser(self, name):
		"""Creates enum type parser."""
		enum_class = self._get_module_class(name)
		string_to_enum = dict(map(lambda key: (key.name.lower(), key), enum_class))

		def parser(enum_value):
			if isinstance(enum_value, str):
				return _name_to_enum_value(string_to_enum, name, enum_value)

			if isinstance(enum_value, int):
				return enum_class(enum_value)

			return enum_value

		self.rules[name] = parser

	def add_struct_parser(self, name):
		"""Creates struct parser (to allow nested parsing)."""
		struct_class = self._get_module_class(name)

		def parser(struct_descriptor):
			struct_processor = self._create_processor(struct_descriptor)
			struct_value = struct_class()

			all_type_hints = _build_type_hints_map(struct_value)
			struct_processor.set_type_hints(all_type_hints)

			struct_processor.copy_to(struct_value)
			return struct_value

		self.rules[f'struct:{name}'] = parser

	def add_array_parser(self, name):
		"""Creates array type parser, based on some existing element type parser."""
		element_rule = self.rules[name]
		element_name = name[len('struct:'):] if name.startswith('struct:') else name

		def parser(values):
			return list(map(element_rule, values))

		self.rules[f'array[{element_name}]'] = parser

	def autodetect(self):
		"""Autodetects rules using reflection."""
		for class_name in dir(self.module):
			cls = getattr(self.module, class_name)
			if not inspect.isclass(cls):
				continue

			if issubclass(cls, BaseValue) and cls != BaseValue:
				self.add_pod_parser(class_name, cls)

			if issubclass(cls, Enum) and cls != Enum and cls != Flag:
				if issubclass(cls, Flag):
					self.add_flags_parser(class_name)
				else:
					self.add_enum_parser(class_name)

	def create_from_factory(self, factory, descriptor):
		"""Creates an entity from a descriptor using a factory."""
		processor = self._create_processor(descriptor)
		entity_type = processor.lookup_value('type')
		entity = factory(entity_type)

		all_type_hints = _build_type_hints_map(entity)
		processor.set_type_hints(all_type_hints)
		processor.copy_to(entity, ['type'])

		self._auto_encode_strings(entity)
		return entity

	def _create_processor(self, descriptor):
		return TransactionDescriptorProcessor(descriptor, self.rules, self.type_converter)

	@staticmethod
	def _auto_encode_strings(entity):
		for key, value in vars(entity).items():
			if isinstance(value, str):
				setattr(entity, key, value.encode('utf8'))
