from catparser.ast import Alias, StructField


def fix_size_name(name):
	return name if 'size' != name else f'{name}_'


def fix_name(name):
	if name in ('type', 'property'):
		return f'{name}_'

	return name


def isinstance_builtin(ast_model, builtin_type):
	return (
		isinstance(ast_model, builtin_type) or (
			isinstance(ast_model, Alias) and isinstance(ast_model.linked_type, builtin_type)
		) or (
			isinstance(ast_model, StructField) and isinstance(ast_model.field_type, builtin_type)
		)
	)
