import re

CAMEL_CASE_PATTERN = re.compile(r'(?<!^)(?=[A-Z])')


def underline_name(name):
	return CAMEL_CASE_PATTERN.sub('_', name).lower()


def fix_size_name(name):
	return name if 'size' != name else f'{name}_'


def fix_name(name):
	if name in ('type', 'property'):
		return f'{name}_'

	return name
