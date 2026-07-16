"""Renders an :class:`AbstractTypeFormatter` to a complete Java source file (one class per file)."""

from .format import LINE_LIMIT, auto_generated_header, indent, javadoc
from .name_formatting import capitalize, uncapitalize


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
	'compareTo': 'Compares this object with another for ordering.',
	'createByName': 'Creates an instance of the concrete type by its name.',
	'fromValue': 'Creates an instance from its underlying numeric value.',
	'parse': 'Parses a raw value into an instance.',
	'has': 'Checks whether a flag is set.',
	'toJson': 'Converts this object to a JSON-serializable representation.',
	'toString': 'Returns a string representation of this object.',
	'equals': 'Compares this object with another for equality.',
	'hashCode': 'Computes the hash code of this object.',
}

_METHOD_RETURNS = {
	'size': 'Size of this object in bytes.',
	'serialize': 'Serialized bytes.',
	'deserialize': 'Deserialized object.',
	'has': 'true if the flag is set, false otherwise.',
	'fromValue': 'Instance corresponding to the numeric value.',
	'parse': 'Parsed instance.',
	'compareTo': 'Negative, zero, or positive as this is less than, equal to, or greater than the other.',
	'toJson': 'JSON-serializable representation of this object.',
	'toString': 'String representation of this object.',
	'equals': 'true if the objects are equal, false otherwise.',
	'hashCode': 'Hash code of this object.',
	'createByName': 'New instance of the named type.',
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
	'rawValue': 'Raw value to parse.',
}


def _argument_description(name):
	# argument names are open-ended (every field name); a capitalized fallback is a fine default
	return _ARGUMENT_DESCRIPTIONS.get(name, f'{capitalize(name)}.')


def _require_doc(mapping, mapping_name, method_name):
	"""Look up a generated method's javadoc text, failing since there is no default"""
	if method_name not in mapping:
		raise RuntimeError(f'no javadoc registered for method {method_name!r} in {mapping_name}; add an entry')

	return mapping[method_name]


def _accessor_property(method_name):
	"""Return the JavaBeans property documented by a getter/setter, or ``None``."""
	for prefix in ('get', 'set'):
		if method_name.startswith(prefix) and len(method_name) > len(prefix) and method_name[len(prefix)].isupper():
			property_name = uncapitalize(method_name[len(prefix):])
			if property_name.endswith('Computed'):
				property_name = property_name[:-len('Computed')]
			return prefix, property_name

	return None


def _default_documentation(method_descriptor, is_constructor, owner_typename):
	"""Build a generic Javadoc for a generated method when the formatter supplied none."""
	# pylint: disable=too-many-locals
	method_name = method_descriptor.method_name
	arguments = method_descriptor.arguments
	result = method_descriptor.result

	accessor = None if is_constructor else _accessor_property(method_name)

	if is_constructor:
		lines = [f'Creates a new {owner_typename}.']
	elif accessor:
		prefix, property_name = accessor
		lines = [f'{"Gets" if "get" == prefix else "Sets"} the {property_name} field.']
	else:
		lines = [_require_doc(_METHOD_SUMMARIES, '_METHOD_SUMMARIES', method_name)]

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
			return_line = f'@return {_require_doc(_METHOD_RETURNS, "_METHOD_RETURNS", method_name)}'

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


def _generate_method(method_descriptor, is_constructor=False, owner_typename=None):
	"""Render a single method to a Java source fragment."""
	parts = []

	# Javadoc — fall back to a generated default for public/protected members so the generated
	# model surface is documented (private helpers and enum constructors are left undocumented).
	documentation = method_descriptor.documentation
	if not documentation and method_descriptor.visibility != 'private':
		documentation = _default_documentation(method_descriptor, is_constructor, owner_typename)
	if documentation:
		parts.append(javadoc(documentation, prefix=''))

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


HEADER = auto_generated_header('run_catbuffer_generator.sh')


def _append_if_not_none(methods, descriptor):
	if descriptor is not None:
		methods.append(descriptor)


def _build_method(name, descriptor, result_default=None):
	"""Render a named method descriptor (defaulting its name/result)."""
	if descriptor is None:
		return None

	if not descriptor.method_name:
		descriptor.method_name = name

	if descriptor.result is None and result_default is not None:
		descriptor.result = result_default

	return _generate_method(descriptor)


def _build_getters_setters(provider):
	return [_generate_method(descriptor) for descriptor in provider.get_getter_setter_descriptors()]


def _build_protected_serializer(provider):
	descriptor = provider.get_serialize_protected_descriptor()
	if descriptor is None:
		return None
	return _generate_method(descriptor)


def _build_core_methods(provider):
	"""Assemble the ordered core-method fragments shared by the class and enum render paths — the single
	source of truth for method ordering. Slots the provider leaves empty (``None``) drop out, so enums
	naturally omit compareTo / sort / the protected serializer while structs and pods include them."""
	methods = []
	_append_if_not_none(methods, _build_method('compareTo', provider.get_compare_to_descriptor(), result_default='int'))
	_append_if_not_none(methods, _build_method('sort', provider.get_sort_descriptor(), result_default='void'))
	methods.extend(_build_getters_setters(provider))
	_append_if_not_none(methods, _build_method('size', provider.get_size_descriptor(), result_default='int'))
	_append_if_not_none(methods, _build_method('deserialize', provider.get_deserialize_descriptor()))
	_append_if_not_none(methods, _build_method('serialize', provider.get_serialize_descriptor(), result_default='byte[]'))
	_append_if_not_none(methods, _build_protected_serializer(provider))
	_append_if_not_none(methods, _build_method('toString', provider.get_str_descriptor(), result_default='String'))
	_append_if_not_none(
		methods,
		_build_method('toJson', provider.get_json_descriptor(), result_default='java.util.Map<String, Object>'))
	return methods


def _file_preamble(package_name):
	"""The shared auto-generated header + package declaration that opens every generated source file."""
	return f'{HEADER}\npackage {package_name};\n\n'


class TypeFormatter:
	"""Assembles a full ``.java`` source file for a POD / Struct / Factory class."""

	def __init__(self, provider, package_name):
		self.provider = provider
		self.package_name = package_name

	def __str__(self):
		return self._render()

	def _render(self):
		# pylint: disable=too-many-locals
		typename = self.provider.typename
		modifiers = 'public ' if self.provider.is_type_abstract else 'public final '

		base = self.provider.get_base_class()
		interfaces = self.provider.get_implemented_interfaces()
		extends = f' extends {base}' if base else ''
		implements = f' implements {interfaces}' if interfaces else ''

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

		for descriptor in self.provider.get_extra_methods():
			method = _generate_method(descriptor, is_constructor=descriptor.is_constructor, owner_typename=typename)
			body += indent(method) + '\n'

		body += '\n'.join(indent(method) for method in _build_core_methods(self.provider))

		header = _file_preamble(self.package_name)
		class_doc = ''
		if self.provider.get_class_documentation():
			class_doc = javadoc(self.provider.get_class_documentation().splitlines(), prefix='')

		return f'{header}{class_doc}{modifiers}class {typename}{extends}{implements}{permits_clause} {{\n{body}}}\n'


class EnumFormatter:
	"""Assembles a full ``.java`` source file for a Java enum (different syntax than a class)."""

	def __init__(self, provider, package_name):
		self.provider = provider
		self.package_name = package_name

	def __str__(self):
		typename = self.provider.typename
		value_type = self.provider.value_type

		constants = self.provider.get_enum_constants()
		const_lines = ',\n\t'.join(constants) + ';'

		fields = f'\tprivate final {value_type} value;\n'

		static_block_lines = self.provider.get_static_block_lines()
		static_block = ('\n' + indent('\n'.join(static_block_lines)) + '\n') if static_block_lines else ''

		ctor_descriptor = self.provider.get_ctor_descriptor()
		ctor = _generate_method(ctor_descriptor, is_constructor=True, owner_typename=typename)

		body = f'\t{const_lines}\n\n{fields}{static_block}\n{indent(ctor)}\n'
		# _build_core_methods is the shared source of method ordering
		body += '\n'.join(indent(method) for method in _build_core_methods(self.provider))

		header = _file_preamble(self.package_name)
		return f'{header}public enum {typename} implements org.symbol.sdk.Serializer {{\n{body}}}\n'
