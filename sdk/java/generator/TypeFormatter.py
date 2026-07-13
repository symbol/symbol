"""Renders an :class:`AbstractTypeFormatter` to a complete Java source file (one class per file)."""

from .format import LINE_LIMIT, indent
from .name_formatting import capitalize, uncapitalize


def _generate_javadoc(documentation_lines):
	if not documentation_lines:
		return ''

	body = '/**\n'
	for line in documentation_lines:
		body += ' *\n' if not line else f' * {line}\n'

	body += ' */\n'
	return body


def _split_argument(argument):
	"""Split a Java argument string (e.g. ``"final byte[] payload"``) into ``(type, name)``."""
	tokens = argument.split()
	name = tokens[-1] if tokens else argument
	type_tokens = tokens[1:-1] if tokens[:1] == ['final'] else tokens[:-1]
	return ' '.join(type_tokens), name


_METHOD_SUMMARIES = {
	'size': 'Calculates the size of the object.',
	'serialize': 'Serializes this object to bytes.',
	'serializeInto': 'Writes this object\'s fields into the supplied buffer.',
	'deserialize': 'Deserializes an instance of this object from a byte payload.',
	'deserializeInto': 'Deserializes this object into an existing instance from a buffer view.',
	'sort': 'Sorts the collections of this object so that it can be serialized canonically.',
	'createByName': 'Creates an instance of the concrete type by its name.',
	'fromValue': 'Creates an instance from its underlying numeric value.',
	'has': 'Checks whether a flag is set.',
	'toJson': 'Converts this object to a JSON-serializable representation.',
	'toString': 'Returns a string representation of this object.',
	'equals': 'Compares this object with another for equality.',
	'hashCode': 'Computes the hash code of this object.',
	'comparer': 'Builds the comparison key for this object.',
}

_METHOD_RETURNS = {
	'size': 'Size of this object in bytes.',
	'serialize': 'Serialized bytes.',
	'deserialize': 'Deserialized object.',
	'has': 'true if the flag is set, false otherwise.',
	'fromValue': 'Instance corresponding to the numeric value.',
	'toJson': 'JSON-serializable representation of this object.',
	'toString': 'String representation of this object.',
	'equals': 'true if the objects are equal, false otherwise.',
	'hashCode': 'Hash code of this object.',
	'comparer': 'Comparison key for this object.',
	'createByName': 'New instance of the named type.',
	'getValue': 'Underlying numeric value.',
}

_ARGUMENT_DESCRIPTIONS = {
	'payload': 'Byte payload to deserialize.',
	'flag': 'Flag bit(s) to test.',
	'buffer': 'Output writer.',
	'view': 'Buffer view to read from; not advanced (the caller moves its cursor by the returned object\'s size()).',
	'cursor': 'Buffer view walking-cursor; advanced past the fields read into the instance.',
	'instance': 'Destination instance to populate.',
	'other': 'Object to compare against.',
	'entityName': 'Entity (transaction) name.',
	'values': 'Discriminator values.',
}


def _argument_description(name):
	return _ARGUMENT_DESCRIPTIONS.get(name, f'{capitalize(name)}.')


def _accessor_property(method_name):
	"""Return the JavaBeans property documented by a getter/setter, or ``None``."""
	for prefix in ('get', 'set'):
		if method_name.startswith(prefix) and len(method_name) > len(prefix) and method_name[len(prefix)].isupper():
			property_name = uncapitalize(method_name[len(prefix):])
			if property_name.endswith('Computed'):
				property_name = property_name[:-len('Computed')]
			return prefix, property_name

	return None


def _default_documentation(method_descriptor, is_constructor, owner_typename):  # pylint: disable=too-many-locals
	"""Build a generic Javadoc for a generated method when the formatter supplied none."""
	method_name = method_descriptor.method_name
	arguments = method_descriptor.arguments
	result = method_descriptor.result

	accessor = None if is_constructor else _accessor_property(method_name)

	if is_constructor:
		lines = [f'Creates a {owner_typename}.']
	elif accessor:
		prefix, property_name = accessor
		lines = [f'{"Gets" if "get" == prefix else "Sets"} the {property_name} field.']
	else:
		lines = [_METHOD_SUMMARIES.get(method_name, f'Invokes {method_name}.')]

	setter_property = accessor[1] if accessor and 'set' == accessor[0] else None

	param_lines = []
	for argument in arguments:
		_, name = _split_argument(argument)
		description = f'New {setter_property} value.' if setter_property and 'value' == name else _argument_description(name)
		param_lines.append(f'@param {name} {description}')

	return_line = None
	if result not in (None, 'void'):
		if accessor and 'get' == accessor[0]:
			return_line = f'@return The {accessor[1]} field.'
		else:
			return_line = f'@return {_METHOD_RETURNS.get(method_name, "Result of the operation.")}'

	if param_lines or return_line:
		lines.append('')
		lines.extend(param_lines)
		if return_line:
			lines.append(return_line)

	# Parse-error contract shared by every deserialize entry point (byte[] and BufferView forms, plus deserializeInto):
	# the two unchecked types malformed input can surface as.
	if method_name in ('deserialize', 'deserializeInto'):
		lines.append('@throws IndexOutOfBoundsException if the buffer is truncated or a declared length overruns it')
		lines.append('@throws IllegalArgumentException if a field carries an invalid value (e.g. a reserved field or unknown discriminator)')

	return lines


def _generate_method(method_descriptor, *, is_constructor=False, owner_typename=None):  # pylint: disable=too-many-locals
	"""Render a single method to a Java source fragment."""
	parts = []

	# Javadoc — fall back to a generated default for public/protected members so the generated
	# model surface is documented (private helpers and enum constructors are left undocumented).
	documentation = method_descriptor.documentation
	if not documentation and method_descriptor.visibility != 'private':
		documentation = _default_documentation(method_descriptor, is_constructor, owner_typename)
	if documentation:
		parts.append(_generate_javadoc(documentation))

	for annotation in method_descriptor.annotations:
		parts.append(annotation + '\n')

	modifiers = []
	if method_descriptor.visibility:
		modifiers.append(method_descriptor.visibility)

	if method_descriptor.is_static and not is_constructor:
		modifiers.append('static')
	modifier_str = ' '.join(modifiers)

	if is_constructor:
		signature = f'{modifier_str} {owner_typename}'.strip()
	else:
		result = method_descriptor.result if method_descriptor.result is not None else 'void'
		signature = f'{modifier_str} {result} {method_descriptor.method_name}'.strip()

	# Arguments — mark every parameter ``final`` and wrap onto separate lines if too long.
	final_arguments = [argument if argument.startswith('final ') else f'final {argument}' for argument in method_descriptor.arguments]
	inline_args = ', '.join(final_arguments)
	if len(signature) + len(inline_args) + 4 > LINE_LIMIT:
		args = '\n\t\t\t' + ',\n\t\t\t'.join(final_arguments) + ')'
		head = f'{signature}({args}'
	else:
		head = f'{signature}({inline_args})'

	body = indent(method_descriptor.body)
	parts.append(f'{head} {{\n{body}}}\n')

	return ''.join(parts)


def _maybe_deserialize_overload(method_descriptor):
	"""Render a public ``deserialize(byte[])`` delegate that wraps the payload in a BufferView for the view-based deserializer."""
	method_name = method_descriptor.method_name
	if 'deserialize' != method_name:
		return None

	stripped = [_split_argument(argument) for argument in method_descriptor.arguments]
	if list(stripped) != [('org.symbol.sdk.utils.BufferView', 'view')]:
		return None

	delegate = type(method_descriptor)(
		method_name=method_name,
		arguments=['byte[] payload'],
		body=f'return {method_name}(new org.symbol.sdk.utils.BufferView(payload));')
	delegate.result = method_descriptor.result
	delegate.visibility = method_descriptor.visibility
	delegate.is_static = method_descriptor.is_static
	# documentation (incl. the shared @throws parse-error contract) is filled in by _default_documentation
	return _generate_method(delegate)


HEADER = (
	'// Auto-generated by sdk/java/generator. Do not edit directly.\n'
	'// Run scripts/run_catbuffer_generator.sh to regenerate.\n'
)


def _append_if_not_none(methods, descriptor):
	if descriptor is not None:
		methods.append(descriptor)


def _build_method(name, descriptor, *, result_default=None):
	"""Render a named method descriptor (defaulting its name/result) plus any ``deserialize(byte[])`` overload."""
	if descriptor is None:
		return None

	if not descriptor.method_name:
		descriptor.method_name = name

	if descriptor.result is None and result_default is not None:
		descriptor.result = result_default

	rendered = _generate_method(descriptor)
	overload = _maybe_deserialize_overload(descriptor)
	if overload:
		rendered = f'{rendered}\n{overload}'

	return rendered


class TypeFormatter:
	"""Assembles a full ``.java`` source file for a POD / Struct / Factory class."""

	def __init__(self, provider, package_name):
		self.provider = provider
		self.package_name = package_name

	def __str__(self):
		return self._render()

	# -- public API ---------------------------------------------------------------------------

	def _render(self):  # pylint: disable=too-many-locals, too-many-statements
		typename = self.provider.typename
		modifiers = 'public final '
		if self.provider.is_type_abstract:
			modifiers = 'public '

		base = self.provider.get_base_class()
		impls = self.provider.get_implements_clause()
		extends = f' extends {base}' if base else ''
		implements = f' implements {impls}' if impls else ''

		# Sealed hierarchies: abstract bases declare their concrete subclasses via `permits` so downstream callers can
		# exhaustive-switch over the closed set.
		permits_list = self.provider.get_permits()
		if permits_list:
			modifiers = 'public sealed '
			permits_clause = ' permits ' + ', '.join(sorted(permits_list))
		else:
			permits_clause = ''

		body = ''

		fields = self.provider.get_fields()
		if fields:
			body += indent('\n'.join(fields)) + '\n'

		ctor_descriptor = self.provider.get_ctor_descriptor()
		if ctor_descriptor is not None:
			ctor_method = _generate_method(ctor_descriptor, is_constructor=True, owner_typename=typename)
			body += indent(ctor_method) + '\n'

		extra = self.provider.get_extra_methods()
		for raw in extra:
			body += raw + '\n'

		methods = []
		_append_if_not_none(methods, _build_method('compareTo', self.provider.get_compare_to_descriptor(), result_default='int'))
		_append_if_not_none(methods, _build_method('sort', self.provider.get_sort_descriptor(), result_default='void'))
		methods.extend(self._build_getters_setters())
		_append_if_not_none(methods, _build_method('size', self.provider.get_size_descriptor(), result_default='int'))
		_append_if_not_none(methods, _build_method('deserialize', self.provider.get_deserialize_descriptor()))
		_append_if_not_none(methods, _build_method('serialize', self.provider.get_serialize_descriptor(), result_default='byte[]'))
		_append_if_not_none(methods, self._build_protected_serializer())
		_append_if_not_none(methods, _build_method('toString', self.provider.get_str_descriptor(), result_default='String'))
		_append_if_not_none(
			methods,
			_build_method('toJson', self.provider.get_json_descriptor(), result_default='java.util.Map<String, Object>'))

		for method in methods:
			body += indent(method) + '\n'

		header = f'{HEADER}\npackage {self.package_name};\n\n'
		class_doc = ''
		if self.provider.get_class_documentation():
			class_doc = _generate_javadoc(self.provider.get_class_documentation().splitlines())

		return f'{header}{class_doc}{modifiers}class {typename}{extends}{implements}{permits_clause} {{\n{body}}}\n'

	# -- helpers ------------------------------------------------------------------------------

	def _build_getters_setters(self):
		out = []
		for descriptor in self.provider.get_getter_setter_descriptors():
			out.append(_generate_method(descriptor))

		return out

	def _build_protected_serializer(self):
		descriptor = self.provider.get_serialize_protected_descriptor()
		if descriptor is None:
			return None

		return _generate_method(descriptor)


class EnumFormatter:
	"""Assembles a full ``.java`` source file for a Java enum (different syntax than a class)."""

	def __init__(self, provider, package_name):
		self.provider = provider
		self.package_name = package_name

	def __str__(self):  # pylint: disable=too-many-locals
		typename = self.provider.typename
		value_type = self.provider.value_type

		constants = self.provider.get_enum_constants()
		const_lines = ',\n\t'.join(constants) + ';'

		fields = f'\tprivate final {value_type} value;\n'

		static_block_lines = []
		if hasattr(self.provider, 'get_static_block_lines'):
			static_block_lines = self.provider.get_static_block_lines()

		static_block = ('\n' + indent('\n'.join(static_block_lines)) + '\n') if static_block_lines else ''

		ctor_descriptor = self.provider.get_ctor_descriptor()
		ctor = _generate_method(ctor_descriptor, is_constructor=True, owner_typename=typename)

		methods = []
		_append_if_not_none(methods, _build_method('size', self.provider.get_size_descriptor()))
		_append_if_not_none(methods, _build_method('serialize', self.provider.get_serialize_descriptor()))
		_append_if_not_none(methods, _build_method('deserialize', self.provider.get_deserialize_descriptor()))

		for descriptor in self.provider.get_getter_setter_descriptors():
			methods.append(_generate_method(descriptor))

		_append_if_not_none(methods, _build_method('toString', self.provider.get_str_descriptor()))
		_append_if_not_none(methods, _build_method('toJson', self.provider.get_json_descriptor()))

		body = f'\t{const_lines}\n\n{fields}{static_block}\n{indent(ctor)}\n'
		for method in methods:
			body += indent(method) + '\n'

		header = f'{HEADER}\npackage {self.package_name};\n\n'
		return f'{header}public enum {typename} implements org.symbol.sdk.Serializer {{\n{body}}}\n'
