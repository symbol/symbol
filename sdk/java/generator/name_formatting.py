"""Naming helpers shared by all Java code-generator formatters.
Java uses lowerCamelCase for fields/methods and UpperCamelCase for types, exactly like JavaScript.
"""

import re

CAMEL_CASE_PATTERN = re.compile(r'(?<!^)(?=[A-Z])')

SNAKE_CASE_PATTERN = re.compile(r'_([a-z])')


def _to_upper(matchobj):
	return matchobj.group(1).upper()


def lang_field_name(name):
	"""Convert a snake_case schema name into Java's lowerCamelCase."""
	return SNAKE_CASE_PATTERN.sub(_to_upper, name)


def capitalize(name):
	"""Uppercase the first character, preserving the rest (unlike ``str.capitalize``)."""
	return name[0].upper() + name[1:] if name else name


def uncapitalize(name):
	"""Lowercase the first character, preserving the rest."""
	return name[0].lower() + name[1:] if name else name


def underline_name(name):
	"""Convert UpperCamelCase into snake_case (used by the parser/printer plumbing)."""
	return CAMEL_CASE_PATTERN.sub('_', name).lower()
