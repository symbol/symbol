"""Emits a Java *typed descriptor* class for each catbuffer ``struct`` AST model."""

from catparser.DisplayType import DisplayType

from .field_filters import descriptor_candidate_fields, is_inherited_field
from .format import _javadoc, _method
from .name_formatting import capitalize, underline_name

# Schema type name -> the hand-written / CryptoTypes Java type used in descriptor signatures
_SDK_TYPE_OVERRIDES = {
	'Address': 'org.symbol.sdk.{blockchain}.Address',
	'UnresolvedAddress': 'org.symbol.sdk.{blockchain}.Address',
	'Hash256': 'org.symbol.sdk.CryptoTypes.Hash256',
	'PublicKey': 'org.symbol.sdk.CryptoTypes.PublicKey',
	'VotingPublicKey': 'org.symbol.sdk.CryptoTypes.PublicKey',
}

# Java boxed types for optional (nullable) primitive parameters.
_PRIMITIVE_BOX = {
	'int': 'Integer',
	'long': 'Long',
	'short': 'Short',
	'byte': 'Byte',
	'boolean': 'Boolean',
}


class DescriptorContext:
	"""Per-blockchain resolution context shared across every descriptor formatter."""

	def __init__(self, blockchain, name_to_display_type_map):
		self.blockchain = blockchain
		self.models_package = f'org.symbol.sdk.{blockchain}.models'
		self.name_to_display_type_map = name_to_display_type_map
		# Per-blockchain sealed interface that gates the descriptor family.
		self.sealed_interface = f'{capitalize(blockchain)}TypedDescriptor'
		# Sealed sub-family implemented only by top-level transaction descriptors used by facades factory
		self.transaction_sealed_interface = f'{capitalize(blockchain)}TransactionDescriptor'

	def has_descriptor(self, type_name):
		"""Returns true when ``type_name`` is a struct that has its own generated descriptor."""
		return DisplayType.STRUCT == self.name_to_display_type_map.get(type_name, DisplayType.UNSET)

	def _sdk_type_override(self, type_name):
		override = _SDK_TYPE_OVERRIDES.get(type_name)
		return override.format(blockchain=self.blockchain) if override is not None else None

	def resolve_named_type(self, type_name):
		"""Resolves a scalar/element schema type name to its Java type (fully qualified)."""
		override = self._sdk_type_override(type_name)
		if override is not None:
			return override

		if self.has_descriptor(type_name):
			return f'{type_name}Descriptor'

		return f'{self.models_package}.{type_name}'

	def string_conversion(self, type_name):
		"""Java expression template (``{value}`` placeholder) converting a String to ``type_name``, or None."""
		override = self._sdk_type_override(type_name)
		if override is not None:
			return f'new {override}({{value}})'

		display_type = self.name_to_display_type_map.get(type_name)
		if display_type in (DisplayType.ENUM, DisplayType.INTEGER, DisplayType.BYTE_ARRAY):
			return f'{self.models_package}.{type_name}.parse({{value}})'

		return None


def _sanitize_comment(comment):
	return ' '.join(str(comment).split())


class DescriptorField:  # pylint: disable=too-many-instance-attributes
	"""Resolved view of one schema field as it appears in a descriptor constructor."""

	def __init__(self, field, context):
		printer = field.extensions.printer
		self.name = printer.name
		self.comment = _sanitize_comment(field.comment)
		self.is_optional = field.display_type.is_array or field.is_conditional
		self._resolve_type(field, context, printer)

	def _resolve_type(self, field, context, printer):
		display_type = field.display_type
		self._unwrap = ''
		# String-form support consumed by the String-form constructor and the optional setters:
		self.string_conversion = None
		self.element_string_conversion = None
		self.descriptor_element_type = None
		if DisplayType.BYTE_ARRAY == display_type:
			self.java_type = 'byte[]'
			self.string_conversion = '{value}.getBytes(java.nio.charset.StandardCharsets.UTF_8)'
			return

		if DisplayType.TYPED_ARRAY == display_type:
			element_name = str(field.field_type.element_type)
			element_type = context.resolve_named_type(element_name)
			self.java_type = f'java.util.List<{element_type}>'
			if context.has_descriptor(element_name):
				self._unwrap = f'.stream().map({element_name}Descriptor::toMap).collect(java.util.stream.Collectors.toList())'
				self.descriptor_element_type = element_type
			else:
				self.element_string_conversion = context.string_conversion(element_name)
			return

		if DisplayType.INTEGER == display_type:
			self.java_type = printer.get_type()
			self.string_conversion = context.string_conversion(str(field.field_type))
		else:
			type_name = str(field.field_type)
			self.java_type = context.resolve_named_type(type_name)
			# Nested struct that has its own typed descriptor: unwrap to its raw map so the
			# rule-based factory receives a Map<String, Object>, matching the typed-array path.
			if context.has_descriptor(type_name):
				self._unwrap = '.toMap()'
			else:
				self.string_conversion = context.string_conversion(type_name)

	def parameter_type(self):
		"""Java parameter type — boxed when optional + primitive so ``null`` is representable."""
		if self.is_optional and self.java_type in _PRIMITIVE_BOX:
			return _PRIMITIVE_BOX[self.java_type]

		return self.java_type

	def store_expression(self):
		"""Java expression stored into the descriptor map for this field."""
		return f'{self.name}{self._unwrap}'

	def convert_expression(self):
		"""Expression converting this field's String parameter to its typed form (scalar fields only)."""
		return self.string_conversion.format(value=self.name)

	def guarded_convert_expression(self):
		"""Like :meth:`convert_expression`, but optional fields pass ``null`` through unconverted."""
		if not self.is_optional:
			return self.convert_expression()

		return f'null == {self.name} ? null : {self.convert_expression()}'


class StructFormatter:
	"""Renders a single ``XxxDescriptor`` Java class (sans package/imports header)."""

	def __init__(self, ast_model, factory_ast_model, context):
		self.struct = ast_model
		self.base_struct = factory_ast_model
		self.context = context

	@property
	def typename(self):
		return f'{self.struct.name}Descriptor'

	def _is_inherited_field(self, field):
		return is_inherited_field(field, self.base_struct)

	def _descriptor_fields(self):
		candidates = descriptor_candidate_fields(self.struct.fields)

		required = []
		optional = []
		for field in candidates:
			if self._is_inherited_field(field):
				continue

			resolved = DescriptorField(field, self.context)
			(optional if resolved.is_optional else required).append(resolved)

		return required, optional

	def render(self):
		required, optional = self._descriptor_fields()
		class_doc = self._render_class_doc()
		fields = '\tprivate final java.util.Map<String, Object> rawDescriptor;\n'
		methods = self._render_constructors(required, optional)
		methods.append(StructFormatter._render_to_map())

		body = f'{fields}\n' + '\n'.join(methods)
		interface = self.context.transaction_sealed_interface if self.base_struct else self.context.sealed_interface
		return (
			f'{class_doc}'
			f'public final class {self.typename} implements {interface} {{\n'
			f'{body}'
			f'}}\n')

	def _render_class_doc(self):
		lines = [f'Type safe descriptor used to generate a descriptor map for {self.typename}.']
		comment = _sanitize_comment(self.struct.comment)
		if comment:
			lines.append('')
			lines.append(comment)
		return _javadoc(lines, prefix='')

	def _render_constructors(self, required, optional):
		methods = [self._render_required_ctor(required)]
		methods.extend(self._render_string_required_ctor(required))
		for field in optional:
			methods.append(self._render_typed_setter(field))
			methods.extend(self._render_setter_overloads(field))

		return methods

	@staticmethod
	def _string_variant_signature(fields):
		if not any(field.string_conversion for field in fields):
			return None

		params = ', '.join(
			f'final String {field.name}' if field.string_conversion else f'final {field.parameter_type()} {field.name}' for field in fields
		)
		args = ', '.join(field.guarded_convert_expression() if field.string_conversion else field.name for field in fields)
		return params, args

	def _render_required_ctor(self, required):
		doc = [f'Creates a descriptor for {self.struct.name}.', '']
		doc.extend(self._param_docs(required))
		params = ', '.join(f'final {field.parameter_type()} {field.name}' for field in required)

		body_lines = ['\t\trawDescriptor = new java.util.LinkedHashMap<>();']
		if self.struct.factory_type:
			body_lines.append(f'\t\trawDescriptor.put("type", "{underline_name(self.struct.name)}");')

		for field in required:
			body_lines.append(f'\t\trawDescriptor.put("{field.name}", {field.store_expression()});')

		body = '\n'.join(body_lines) + '\n'
		return _method(doc, f'\tpublic {self.typename}({params})', body)

	def _render_string_required_ctor(self, required):
		signature = StructFormatter._string_variant_signature(required)
		if signature is None:
			return []

		params, args = signature
		doc = [f'Creates a descriptor for {self.struct.name} from string-form values.', '']
		doc.extend(self._param_docs(required))
		return [_method(doc, f'\tpublic {self.typename}({params})', f'\t\tthis({args});\n')]

	def _render_typed_setter(self, field):
		signature = f'\tpublic {self.typename} {field.name}(final {field.parameter_type()} {field.name})'
		body = (
			f'\t\tif (null != {field.name})\n'
			f'\t\t\trawDescriptor.put("{field.name}", {field.store_expression()});\n'
			f'\t\treturn this;\n')
		return _method(self._setter_doc(field), signature, body)

	def _render_setter_overloads(self, field):
		doc = self._setter_doc(field)
		signature = f'\tpublic {self.typename} {field.name}'
		if field.string_conversion:
			body = f'\t\treturn {field.name}({field.guarded_convert_expression()});\n'
			return [_method(doc, f'{signature}(final String {field.name})', body)]

		if field.element_string_conversion:
			element_expression = field.element_string_conversion.format(value='value')
			body = (
				f'\t\treturn {field.name}(java.util.Arrays.stream({field.name}).map(value -> {element_expression})'
				f'.collect(java.util.stream.Collectors.toList()));\n')
			return [_method(doc, f'{signature}(final String... {field.name})', body)]

		if field.descriptor_element_type:
			body = f'\t\treturn {field.name}(java.util.Arrays.asList({field.name}));\n'
			return [_method(doc, f'{signature}(final {field.descriptor_element_type}... {field.name})', body)]

		return []

	@staticmethod
	def _setter_doc(field):
		return [
			f'Sets the {field.name} field.',
			'',
			f'@param {field.name} {field.comment or field.name}',
			'@return This descriptor for chaining.',
		]

	@staticmethod
	def _render_to_map():
		doc = [
			'Builds a representation of this descriptor that can be passed to a factory function.',
			'',
			'@return Descriptor map that can be passed to a transaction factory.'
		]
		body = '\t\treturn rawDescriptor;\n'
		return _method(doc, '\t@Override\n\tpublic java.util.Map<String, Object> toMap()', body)

	@staticmethod
	def _param_docs(fields):
		docs = []
		for field in fields:
			comment = field.comment or field.name
			docs.append(f'@param {field.name} {comment}')

		return docs
