"""Emits the generated per-descriptor sweep test."""

from catparser.DisplayType import DisplayType

from .DescriptorStructTypeFormatter import DescriptorField
from .field_filters import descriptor_candidate_fields, is_inherited_field
from .ModelsSweepTestFormatter import resolve_concrete_standin
from .name_formatting import capitalize, uncapitalize, underline_name

_SAMPLE_ADDRESSES = {
	'symbol': 'TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I',
	'nem': 'TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C',
}

# Deterministic signer used by the factory round-trip.
_SAMPLE_SIGNER = '364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28'

_FACTORY_ASSIGNED_FIELDS = frozenset({
	('NamespaceRegistrationTransactionV1', 'id'),
	('MosaicDefinitionTransactionV1', 'id'),
})


class _FieldSample:
	"""Deterministic sample expressions for one descriptor field (typed / string / expected forms)."""
	# pylint: disable=too-many-instance-attributes

	def __init__(self, raw_field, resolved, context, type_info, index, owner_name):
		# pylint: disable=too-many-arguments,too-many-positional-arguments
		self.resolved = resolved
		self.name = resolved.name
		self.index = index
		self.is_conditional = raw_field.is_conditional
		self.owner_name = owner_name
		self._raw_field = raw_field
		self._context = context
		self._type_info = type_info
		self.element_typed_expr = None
		# model-struct samples are fresh instances without value equality; their asserts degrade to non-null
		self.is_model_struct = False
		# lists of model-only struct elements: no element equality either, but size is assertable
		self.is_struct_element_list = False
		self.struct_element_concrete = None
		self.typed_expr = self._make_typed_expr()

	def _sample_string_for(self, type_name):
		"""The canonical sample string for a named scalar type; positionally distinct where possible
		(addresses are fixed valid literals; enums rotate constants by field position)."""
		if type_name in ('Address', 'UnresolvedAddress'):
			return _SAMPLE_ADDRESSES[self._context.blockchain]

		enum_value = self._type_info.enum_value(type_name, self.index - 1)
		if enum_value is not None:
			return enum_value

		byte_size = self._type_info.byte_array_size(type_name)
		if byte_size is not None:
			return f'{self.index % 255 + 1:02X}' * byte_size

		return str(self.index)

	def _make_typed_expr(self):
		"""Returns the typed Java expression for this field, recording sample string literals as a side effect."""
		# pylint: disable=too-many-return-statements
		resolved = self.resolved
		if 'byte[]' == resolved.java_type:
			# the field's own name doubles as a distinct payload
			return f'"{self.name}".getBytes(java.nio.charset.StandardCharsets.UTF_8)'

		if resolved.string_conversion:
			sample = self._sample_string_for(str(self._raw_field.field_type))
			return resolved.string_conversion.format(value=f'"{sample}"')

		if resolved.descriptor_element_type:
			element = resolved.descriptor_element_type
			return f'java.util.List.of(sweepNew{element[:-len("Descriptor")]}Descriptor())'

		if resolved.element_string_conversion:
			element_name = str(self._raw_field.field_type.element_type)
			self.element_typed_expr = resolved.element_string_conversion.format(value=f'"{self._sample_string_for(element_name)}"')
			return f'java.util.List.of({self.element_typed_expr})'

		if resolved.java_type.startswith('java.util.List<'):
			self.is_struct_element_list = True
			element_name = str(self._raw_field.field_type.element_type)
			concrete = self._type_info.concrete_standin(element_name)
			self.struct_element_concrete = f'{self._context.models_package}.{concrete}'
			return f'java.util.List.of(new {self.struct_element_concrete}())'

		if resolved.java_type.endswith('Descriptor'):
			return f'sweepNew{resolved.java_type[:-len("Descriptor")]}Descriptor()'

		if resolved.java_type in ('int', 'long', 'Integer', 'Long'):
			suffix = 'L' if resolved.java_type in ('long', 'Long') else ''
			return f'{self.index}{suffix}'

		# model-only struct (possibly abstract): concrete stand-in default instance (AST-derived)
		self.is_model_struct = True
		type_name = str(self._raw_field.field_type)
		standin = self._type_info.concrete_standin(type_name)
		return f'new {self._context.models_package}.{standin}()'

	def map_expected_expr(self):
		"""Expression equal to the value ``toMap()`` stores for this field's sample."""
		if self.resolved.descriptor_element_type:
			element = self.resolved.descriptor_element_type
			return f'java.util.List.of(sweepNew{element[:-len("Descriptor")]}Descriptor().toMap())'

		if self.resolved.java_type.endswith('Descriptor') and not self.resolved.java_type.startswith('java.util'):
			return f'{self.typed_expr}.toMap()'

		return self.typed_expr

	def model_expected(self):
		"""(java expression, is_exact) the created model's ``getField`` must return, or (None, False) to
		assert non-null only (structs / lists without value equality)."""
		if (self.owner_name, self.name) in _FACTORY_ASSIGNED_FIELDS:
			return None, False

		resolved = self.resolved
		if 'byte[]' == resolved.java_type:
			return self.typed_expr, True

		if resolved.string_conversion:
			type_name = str(self._raw_field.field_type)
			return self._convert_scalar_expected(type_name, self.typed_expr), True

		if resolved.element_string_conversion:
			element_name = str(self._raw_field.field_type.element_type)
			return f'java.util.List.of({self._convert_scalar_expected(element_name, self.element_typed_expr)})', True

		if resolved.java_type in ('int', 'long', 'Integer', 'Long'):
			return self.typed_expr, True

		return None, False

	def _convert_scalar_expected(self, type_name, inner_expr):
		"""The model-side value the factory rules produce for a sample scalar (per-network conversions applied)."""
		if type_name in ('Address', 'UnresolvedAddress'):
			if 'nem' == self._context.blockchain:
				encoded = f'({inner_expr}).toString().getBytes(java.nio.charset.StandardCharsets.UTF_8)'
				return f'new {self._context.models_package}.Address({encoded})'

			return f'new {self._context.models_package}.{type_name}(({inner_expr}).bytes())'

		if type_name in ('Hash256', 'PublicKey', 'VotingPublicKey'):
			# CryptoTypes value bridges to its same-named model twin via asByteArray
			return f'new {self._context.models_package}.{type_name}(({inner_expr}).bytes())'

		return inner_expr


class _TypeInfo:
	"""Schema-wide lookups the sample synthesis needs (enum constants, pod byte sizes, abstract stand-ins)."""

	def __init__(self, ast_models):
		self._enum_values = {}
		self._byte_array_size = {}
		self.abstract_child = {}
		self.abstract_names = set()
		for model in ast_models:
			if DisplayType.ENUM == model.display_type and model.values:
				self._enum_values[model.name] = [entry.name.lower() for entry in model.values]
			elif DisplayType.BYTE_ARRAY == model.display_type:
				self._byte_array_size[model.name] = model.size
			elif DisplayType.STRUCT == model.display_type:
				if model.is_abstract:
					self.abstract_names.add(model.name)
				elif model.factory_type:
					self.abstract_child.setdefault(model.factory_type, model.name)

	def concrete_standin(self, type_name):
		return resolve_concrete_standin(self.abstract_names, self.abstract_child, type_name)

	def enum_value(self, type_name, position):
		"""The constant name at ``position`` (mod the constant count), so sibling fields rotate constants."""
		values = self._enum_values.get(type_name)
		return values[position % len(values)] if values else None

	def byte_array_size(self, type_name):
		return self._byte_array_size.get(type_name)


class SweepTestFormatter:
	"""Renders ``DescriptorsSweepTest.java`` from the resolved descriptor struct formatters."""

	def __init__(self, struct_entries, context, ast_models):
		# struct_entries: list of (ast_model, factory_ast_model, StructFormatter)
		self.struct_entries = struct_entries
		self.context = context
		self.type_info = _TypeInfo(ast_models)

	def _factory_typename(self):
		return f'org.symbol.sdk.{self.context.blockchain}.{capitalize(self.context.blockchain)}TransactionFactory'

	def _field_samples(self, ast_model, factory_ast_model):
		"""Resolve each candidate field and partition into (required, optional) samples, preserving order."""
		required = []
		optional = []
		index = 0
		for raw_field in descriptor_candidate_fields(ast_model.fields):
			if is_inherited_field(raw_field, factory_ast_model):
				continue

			index += 1
			resolved = DescriptorField(raw_field, self.context)
			sample = _FieldSample(raw_field, resolved, self.context, self.type_info, index, owner_name=ast_model.name)
			(optional if resolved.is_optional else required).append(sample)

		return required, optional

	@staticmethod
	def _helper_name(typename):
		return f'sweepNew{typename}'

	def _render_helper(self, typename, required, optional):
		arguments = ', '.join([sample.typed_expr for sample in required] + ['null' for _ in optional])
		doc = 'distinct sample required arguments and null optionals' if optional else 'distinct sample required arguments'
		return (
			f'\t/** Constructs a {typename} with {doc}. */\n'
			f'\tprivate static {typename} {self._helper_name(typename)}() {{\n'
			f'\t\treturn new {typename}({arguments});\n'
			'\t}\n')

	@staticmethod
	def _full_ctor_expr(typename, required, optional):
		"""All-args constructor expression with every field populated by its sample."""
		args = ', '.join(sample.typed_expr for sample in required + optional)
		return f'new {typename}({args})'

	@staticmethod
	def _render_construction_asserts(ast_model, required, optional):
		lines = []
		if ast_model.factory_type:
			lines.append(f'\t\tassertThat(built.get("type"), equalTo("{underline_name(ast_model.name)}"));')

		for sample in required:
			if sample.is_model_struct:
				lines.append(f'\t\tassertThat(built.get("{sample.name}"), notNullValue());')
			else:
				lines.append(f'\t\tassertThat(built.get("{sample.name}"), equalTo({sample.map_expected_expr()}));')

		for sample in optional:
			lines.append(f'\t\tassertThat(built.containsKey("{sample.name}"), equalTo(false));')

		lines.append(f'\t\tassertThat(built.size(), equalTo({len(required) + (1 if ast_model.factory_type else 0)}));')
		return lines

	def _render_full_ctor_asserts(self, ast_model, typename, required, optional):
		"""Asserts the all-args constructor stores every optional field (the required half is covered above)."""
		if not optional:
			return []

		lines = [f'\t\tfinal java.util.Map<String, Object> fullBuilt = {self._full_ctor_expr(typename, required, optional)}.toMap();']
		for sample in optional:
			if sample.is_model_struct:
				lines.append(f'\t\tassertThat(fullBuilt.get("{sample.name}"), notNullValue());')
			elif sample.is_struct_element_list:
				# struct elements have no value equality; the one-element size distinguishes the stored
				# list from an empty default
				lines.append(f'\t\tassertThat(((java.util.List<?>) fullBuilt.get("{sample.name}")).size(), equalTo(1));')
			else:
				lines.append(f'\t\tassertThat(fullBuilt.get("{sample.name}"), equalTo({sample.map_expected_expr()}));')

		total = len(required) + len(optional) + (1 if ast_model.factory_type else 0)
		lines.append(f'\t\tassertThat(fullBuilt.size(), equalTo({total}));')
		return lines

	def _render_factory_round_trip_test(self, ast_model, typename, required, optional):
		"""Standalone factory round-trip test: the fully-populated map creates a model whose fields echo the samples."""
		# pylint: disable=too-many-locals
		if not ast_model.factory_type:
			return None

		# struct-element lists get a held element local so the created model can be asserted to carry
		# the exact same instance (the pipeline has no conversion rule for these element types)
		element_locals = {}
		arguments = [sample.typed_expr for sample in required]
		for sample in optional:
			if sample.is_struct_element_list:
				local = f'sweep{capitalize(sample.name)}Element'
				element_locals[sample.name] = local
				arguments.append(f'java.util.List.of({local})')
			else:
				arguments.append(sample.typed_expr)

		descriptor_expr = f'new {typename}({", ".join(arguments)})'
		concrete_model = f'{self.context.models_package}.{ast_model.name}'
		lines = ['\t\t// Arrange: the sample descriptor with every field populated, plus the signer']
		for sample in optional:
			if sample.name in element_locals:
				lines.append(
					f'\t\tfinal {sample.struct_element_concrete} {element_locals[sample.name]}'
					f' = new {sample.struct_element_concrete}();')

		lines.extend([
			f'\t\tfinal java.util.Map<String, Object> createMap = sweepCreateMap({descriptor_expr});',
			'',
			'\t\t// Act:',
			f'\t\tfinal {concrete_model} transaction = ({concrete_model}) FACTORY.create(createMap);',
			'',
			'\t\t// Assert: every created-model field echoes its sample (per-network conversions applied)',
		])
		for sample in required + optional:
			# conditional non-byte[] getters wrap in Optional
			unwrap = sample.is_conditional and 'byte[]' != sample.resolved.java_type
			expected, is_exact = sample.model_expected()
			if is_exact:
				getter = f'transaction.get{capitalize(sample.name)}(){".get()" if unwrap else ""}'
				lines.append(f'\t\tassertThat({getter}, equalTo({expected}));')
			elif unwrap:
				# Optional.get() before a null-check is tautological; assert presence and fail cleanly
				lines.append(f'\t\tassertThat(transaction.get{capitalize(sample.name)}().isPresent(), equalTo(true));')
			elif sample.is_struct_element_list:
				# the created model must carry exactly our element instance (no conversion rule applies)
				lines.append(f'\t\tassertThat(transaction.get{capitalize(sample.name)}().size(), equalTo(1));')
				lines.append(
					f'\t\tassertThat(transaction.get{capitalize(sample.name)}().get(0),'
					f' sameInstance({element_locals[sample.name]}));')
			else:
				lines.append(f'\t\tassertThat(transaction.get{capitalize(sample.name)}(), notNullValue());')

		# factory-built transactions are semantically valid, so unlike the models sweep's synthetic
		# instances the wire round trip is asserted unconditionally (byte identity stands in for
		# instance equality — generated models have no equals)
		lines.extend([
			'',
			'\t\t// Assert: the created transaction round-trips through the wire form byte-identically',
			'\t\tfinal byte[] payload = transaction.serialize();',
			'\t\tassertThat(payload.length, equalTo(transaction.size()));',
			f'\t\tassertThat({self._factory_typename()}.deserialize(payload).serialize(), equalTo(payload));',
		])

		body = '\n'.join(lines)
		return (
			'\t@Test\n'
			f'\tvoid {uncapitalize(typename)}FactoryRoundTrip() {{\n'
			f'{body}\n'
			'\t}\n')

	def _render_test(self, ast_model, factory_ast_model, formatter):
		typename = formatter.typename
		required, optional = self._field_samples(ast_model, factory_ast_model)

		assert_comment = '\t\t// Assert: exact required contents'
		if optional:
			assert_comment += '; null optional arguments leave their keys absent'

		body_lines = [
			'\t\t// Arrange:',
			f'\t\tfinal {typename} descriptor = {self._helper_name(typename)}();',
			'',
			'\t\t// Act:',
			'\t\tfinal java.util.Map<String, Object> built = descriptor.toMap();',
			'',
			assert_comment,
		]
		body_lines.extend(SweepTestFormatter._render_construction_asserts(ast_model, required, optional))
		full_ctor_lines = self._render_full_ctor_asserts(ast_model, typename, required, optional)
		if full_ctor_lines:
			body_lines.append('')
			body_lines.append('\t\t// Act + Assert: the all-args constructor stores every optional field')
			body_lines.extend(full_ctor_lines)

		body = '\n'.join(body_lines)
		parts = [
			f'{self._render_helper(typename, required, optional)}\n'
			'\t@Test\n'
			f'\tvoid {uncapitalize(typename)}() {{\n'
			f'{body}\n'
			'\t}\n'
		]
		round_trip_test = self._render_factory_round_trip_test(ast_model, typename, required, optional)
		if round_trip_test is not None:
			parts.append(round_trip_test)

		return '\n'.join(parts)

	def render(self):
		factory_typename = self._factory_typename()
		network_typename = f'org.symbol.sdk.{self.context.blockchain}.Network'
		transaction_interface = self.context.transaction_sealed_interface
		tests = '\n'.join(
			self._render_test(ast_model, factory_ast_model, formatter)
			for ast_model, factory_ast_model, formatter in self.struct_entries)
		return (
			'/**\n'
			' * Generated per-descriptor sweep: construction with distinct sample values, exact {@code toMap()} contents\n'
			' * (null optional arguments leave their keys absent), all-args constructor storage of every field, and\n'
			' * (for transaction descriptors) a factory round-trip verifying each created-model field via {@code getField}.\n'
			' */\n'
			'final class DescriptorsSweepTest {\n'
			'\tprivate static final org.symbol.sdk.CryptoTypes.PublicKey SWEEP_SIGNER_PUBLIC_KEY ='
			f' new org.symbol.sdk.CryptoTypes.PublicKey("{_SAMPLE_SIGNER}");\n'
			'\n'
			f'\tprivate static final {factory_typename} FACTORY = new {factory_typename}({network_typename}.TESTNET);\n'
			'\n'
			'\t/**\n'
			'\t * Returns the descriptor\'s map with the sweep signer added. No defensive copy is needed: toMap() returns\n'
			'\t * the live descriptor map, and the descriptor is a throwaway sweep instance.\n'
			'\t */\n'
			f'\tprivate static java.util.Map<String, Object> sweepCreateMap(final {transaction_interface} descriptor) {{\n'
			'\t\tfinal java.util.Map<String, Object> createMap = descriptor.toMap();\n'
			'\t\tcreateMap.put("signerPublicKey", SWEEP_SIGNER_PUBLIC_KEY);\n'
			'\t\treturn createMap;\n'
			'\t}\n'
			'\n'
			f'{tests}'
			'}\n')
