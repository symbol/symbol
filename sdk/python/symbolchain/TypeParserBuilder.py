import functools
import importlib
import inspect
import operator

from .BaseValue import BaseValue
from .TransactionDescriptorProcessor import TransactionDescriptorProcessor


def _name_to_enum_value(mapping, enum_type, enum_value_name):
	if enum_value_name not in mapping:
		raise RuntimeError(f'unknown value {enum_value_name} for type {enum_type}')
	return mapping[enum_value_name]


def build_type_hints_map(transaction):
	type_hints = {}
	for key, hint in transaction.TYPE_HINTS.items():
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


class TypeParserBuilder:
	"""Catbuffer type parser builder."""

	def __init__(self, name, type_converter, base_type_parsing_rules=None):
		"""Creates type parser builder for catbuffer module"""
		self.module = importlib.import_module(f'symbolchain.{name}')
		self.type_converter = type_converter
		self.base_type_parsing_rules = base_type_parsing_rules

	def _get_catbuffer_class(self, name):
		return getattr(self.module, name)

	def create_flags_parser(self, name):
		"""Creates flag type parser"""
		cls = self._get_catbuffer_class(name)
		string_to_enum = dict(map(lambda key: (key.name.lower(), key), cls))

		def parser(flags):
			if isinstance(flags, str):
				enum_array = list(map(lambda flag_name: _name_to_enum_value(string_to_enum, name, flag_name), flags.split(' ')))
				return functools.reduce(operator.or_, enum_array)

			if isinstance(flags, int):
				return cls(flags)

			return flags

		return parser

	def create_enum_parser(self, name):
		"""Creates enum type parser."""
		cls = self._get_catbuffer_class(name)
		string_to_enum = dict(map(lambda key: (key.name.lower(), key), cls))

		def parser(enum_value):
			if isinstance(enum_value, str):
				return _name_to_enum_value(string_to_enum, name, enum_value)

			if isinstance(enum_value, int):
				return cls(enum_value)

			return enum_value

		return parser

	@staticmethod
	def create_object_array_parser(name, existing_rules):
		"""Creates array type parser, based on some existing element type parser."""
		element_rule = existing_rules[name]

		def parser(values):
			return list(map(element_rule, values))

		return parser

	def create_struct_parser(self, struct_name, extend_type_parsing_rules):
		"""Creates struct parser (to allow nested parsing)."""
		struct_class = self._get_catbuffer_class(struct_name)

		def parser(struct_descriptor):
			extended_type_parsing_rules = extend_type_parsing_rules(self.type_converter, self.base_type_parsing_rules)
			struct_processor = TransactionDescriptorProcessor(struct_descriptor, extended_type_parsing_rules, self.type_converter)
			struct_obj = struct_class()

			all_type_hints = build_type_hints_map(struct_obj)
			struct_processor.set_type_hints(all_type_hints)

			struct_processor.copy_to(struct_obj)
			return struct_obj

		return parser

	def create_sdk_wrapper(self, typename):
		"""Creates wrapper for SDK types."""
		if self.base_type_parsing_rules and typename in self.base_type_parsing_rules:
			return self.base_type_parsing_rules[typename]

		return lambda e: typename(e) if not isinstance(e, typename) else e

	def add_base_value_rules(self, rules):
		"""Adds parsers for all base values from module."""
		for class_name in dir(self.module):
			cls = getattr(self.module, class_name)
			if inspect.isclass(cls):
				if issubclass(cls, BaseValue) and cls != BaseValue:
					rules[class_name] = cls
