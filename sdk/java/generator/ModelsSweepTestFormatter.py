"""Emits the generated per-model sweep test (the reflection-free replacement for the old
``ModelsFieldAccessSweepTest``).

For each generated type the emitted ``ModelsSweepTest.java`` contains one explicit ``@Test`` method:
structs drive every typed property setter/getter pair with a distinct typed value (conditional getters
unwrap their ``Optional``), cross-check ``getField`` against the typed getter, and then drive the
descriptor-pipeline ``setField`` arm with a second distinct value read back through the typed getter —
so both field-access surfaces of every model (including the receipt/block/statement models the factory
round-trip tests never reach) are executed and discriminated; pods, enums and flags assert their exact
value surface (``value``/``size``/``toString``/``toJson``/``serialize`` against schema-derived literals,
for every constant) and every generated ``parse`` arm. Struct-level ``size``/``toString``/``toJson``
stay smoke-only: their expected values are runtime-recursive, and the cross-SDK catbuffer vectors own
the real serialization guarantees. Deterministic emission; unhandled field kinds fail generation loudly.
"""

from catparser.DisplayType import DisplayType

from .field_filters import is_computed
from .name_formatting import capitalize, uncapitalize


def resolve_concrete_standin(abstract_names, abstract_child, type_name):
	"""Concrete class to instantiate for ``type_name`` (the registered child when it is abstract).

	Fails generation for a childless abstract instead of silently synthesizing a bare base instance.
	"""
	if type_name in abstract_names:
		concrete = abstract_child.get(type_name)
		if concrete is None:
			raise RuntimeError(f'{type_name} is abstract but has no concrete child to use as a sweep sample')
		return concrete
	return type_name


_STRUCT_SKIP_SUFFIX = 'Factory'

# High-bit, hex-alphabetic sample values: a case-flip, sign-extension, or byte-order regression must
# change the rendered form.
_UNSIGNED_POD_SAMPLE = 163  # 0xA3
_SIGNED_POD_SAMPLE = 106  # 0x6A (high bit unavailable for signed 1-byte pods)
_PATTERN_SEED = 0xA0

# setField second-sample offset; keeps primitive samples inside the JLS-mandated box cache (<= 127),
# which the sameInstance cross-checks rely on.
_SECOND_SAMPLE_OFFSET = 50
_BOX_CACHE_LIMIT = 127


def _le_bytes_literal(value, size):
	rendered = ', '.join(f'(byte) 0x{(value >> (8 * i)) & 0xFF:02X}' for i in range(size))
	return f'new byte[]{{{rendered}}}'


def _int_literal(value, size, is_unsigned):
	is_long = 8 == size or (4 == size and is_unsigned)
	if value >= 2**63:
		return f'0x{value:X}L'

	return f'{value}L' if is_long else f'{value}'


def _to_json_literal(size, quoted, numeric):
	return quoted if 8 == size else numeric


def _pattern_hex(size, seed):
	return ''.join(f'{(seed + i) & 0xFF:02X}' for i in range(size))


class _ModelTypeInfo:
	"""Schema-wide lookups for struct-field value synthesis."""

	def __init__(self, ast_models):
		self.pod_int = {}
		self.pod_bytes = {}
		self.enum_names = {}
		self.flags_names = {}
		self.abstract_child = {}
		self.abstract_names = set()
		self.struct_names = set()
		for model in ast_models:
			if DisplayType.ENUM == model.display_type:
				names = [entry.name for entry in model.values]
				if model.is_bitwise:
					self.flags_names[model.name] = names
				else:
					self.enum_names[model.name] = names
			elif DisplayType.INTEGER == model.display_type:
				self.pod_int[model.name] = model.size
			elif DisplayType.BYTE_ARRAY == model.display_type:
				self.pod_bytes[model.name] = model.size
			elif DisplayType.STRUCT == model.display_type:
				self.struct_names.add(model.name)
				if model.is_abstract:
					self.abstract_names.add(model.name)
				elif model.factory_type and model.factory_type not in self.abstract_child:
					self.abstract_child[model.factory_type] = model.name

	def concrete_standin(self, type_name):
		return resolve_concrete_standin(self.abstract_names, self.abstract_child, type_name)

	def _constant_names(self, type_name):
		return self.enum_names.get(type_name) or self.flags_names.get(type_name)

	def first_constant(self, type_name):
		names = self._constant_names(type_name)
		return names[0] if names else None

	def second_constant(self, type_name):
		"""A constant distinct from the first when one exists (single-constant types fall back to the first)."""
		names = self._constant_names(type_name)
		if not names:
			return None

		return names[1] if 1 < len(names) else names[0]


class ModelsSweepTestFormatter:
	"""Renders ``ModelsSweepTest.java`` from the generated model formatters."""

	def __init__(self, entries, ast_models):
		# entries: list of (ast_model, formatter); formatter is the model StructTypeFormatter etc.
		self.entries = entries
		self.type_info = _ModelTypeInfo(ast_models)

	def _field_value(self, field, index, second=False):
		# pylint: disable=too-many-return-statements
		printer = field.extensions.printer
		type_name = str(field.field_type)
		suffix = '-2' if second else ''
		offset = _SECOND_SAMPLE_OFFSET if second else 0
		if field.display_type.is_array and 'byte[]' == printer.get_type():
			return 'byte[]', f'"{printer.name}{suffix}".getBytes(java.nio.charset.StandardCharsets.UTF_8)', 'bytes'

		if field.display_type.is_array:
			# mutable: the generated sort() sorts ordered arrays in place (List.of() would throw)
			return printer.get_type(), 'new java.util.ArrayList<>()', 'list'

		if type_name in self.type_info.pod_int:
			return type_name, f'new {type_name}({index + offset}L)', 'reference'

		if type_name in self.type_info.pod_bytes:
			return type_name, f'new {type_name}(sweepBytes({type_name}.SIZE, {index + offset}))', 'reference'

		constant = self.type_info.second_constant(type_name) if second else self.type_info.first_constant(type_name)
		if constant is not None:
			return type_name, f'{type_name}.{constant}', 'reference'

		if type_name in self.type_info.struct_names:
			concrete = self.type_info.concrete_standin(type_name)
			return type_name, f'new {concrete}()', 'reference'

		java_type = printer.get_type()
		if java_type in ('int', 'long'):
			sample = index + offset
			# the sameInstance getField cross-check needs both boxings to hit the shared cache
			assert sample <= _BOX_CACHE_LIMIT, f'primitive sample {sample} exceeds the Integer/Long box cache'
			literal_suffix = 'L' if 'long' == java_type else ''
			return java_type, f'{sample}{literal_suffix}', 'primitive'

		raise RuntimeError(f'no sweep sample strategy for field "{printer.name}" of type {type_name} ({java_type})')

	def _render_struct_test(self, ast_model, formatter):
		# pylint: disable=too-many-locals
		typename = ast_model.name
		lines = [
			'\t\t// Arrange:',
			f'\t\tfinal {typename} instance = new {typename}();',
			'',
			'\t\t// Act + Assert: each typed property pair, cross-checked against the descriptor-pipeline',
			'\t\t// getField, then re-driven through the setField arm with a second distinct value',
		]
		swept_fields = [field for field in formatter.non_reserved_fields(include_inherited=True) if not is_computed(field)]
		for index, field in enumerate(swept_fields, start=1):
			local_type, value_expr, kind = self._field_value(field, index)
			_, second_expr, _ = self._field_value(field, index, second=True)

			name = field.extensions.printer.name
			accessor = capitalize(name)
			var = f'value{index}'
			# conditional non-byte[] getters wrap in Optional; byte[] getters return the stored byte[] directly
			unwrap = field.is_conditional and 'bytes' != kind
			getter = f'instance.get{accessor}().get()' if unwrap else f'instance.get{accessor}()'
			same_matcher = kind in ('reference', 'bytes')
			lines.append(f'\t\tfinal {local_type} {var} = {value_expr};')
			lines.append(f'\t\tinstance.set{accessor}({var});')
			lines.append(f'\t\tassertThat({getter}, {"sameInstance" if same_matcher else "equalTo"}({var}));')
			# the descriptor-pipeline getField must agree with the typed getter: same reference for every stored
			# kind (primitive samples stay inside the JLS-mandated box cache; byte[] is stored by reference)
			lines.append(f'\t\tassertThat(instance.getField("{name}"), sameInstance({getter}));')
			lines.append(f'\t\tfinal {local_type} {var}b = {second_expr};')
			lines.append(f'\t\tinstance.setField("{name}", {var}b);')
			lines.append(f'\t\tassertThat({getter}, {"sameInstance" if same_matcher else "equalTo"}({var}b));')

		lines.append('')
		lines.append('\t\t// Assert: surface smoke — expected values are runtime-recursive; vectors own serialization')
		lines.append('\t\tinstance.sort();')
		lines.append('\t\tassertDoesNotThrow(instance::toString);')
		lines.append('\t\tassertDoesNotThrow(instance::toJson);')
		if any(field.is_conditional for field in swept_fields):
			# conditional-null defaults may legitimately reject serialization, so size() is smoke-checked
			# standalone and the size == length cross-check applies only when serialize succeeds
			lines.append('\t\tassertDoesNotThrow(instance::size);')
			lines.append('\t\ttry {')
			lines.append('\t\t\tfinal byte[] payload = instance.serialize();')
			lines.append('\t\t\tassertThat(payload.length, equalTo(instance.size()));')
			lines.append(f'\t\t\tassertThat({typename}.deserialize(java.nio.ByteBuffer.wrap(payload)).serialize(), equalTo(payload));')
			lines.append('\t\t} catch (RuntimeException ignored) {')
			lines.append('\t\t\t// conditional-null default rejected; vectors cover valid payloads')
			lines.append('\t\t}')
		else:
			lines.append('\t\tfinal byte[] payload = instance.serialize();')
			lines.append('\t\tassertThat(payload.length, equalTo(instance.size()));')
			lines.append('\t\ttry {')
			lines.append(f'\t\t\tassertThat({typename}.deserialize(java.nio.ByteBuffer.wrap(payload)).serialize(), equalTo(payload));')
			lines.append('\t\t} catch (RuntimeException ignored) {')
			lines.append('\t\t\t// synthetic sample rejected on the read path; vectors cover valid payloads')
			lines.append('\t\t}')

		return self._render_method(typename, lines)

	@staticmethod
	def _render_int_pod_test(ast_model):
		typename = ast_model.name
		size = ast_model.size
		sample = _UNSIGNED_POD_SAMPLE if ast_model.is_unsigned else _SIGNED_POD_SAMPLE
		# 1-byte signed pods cap at 127
		if not ast_model.is_unsigned and 1 == size:
			sample = _SIGNED_POD_SAMPLE
		to_string = f'0x{sample:0{2 * size}X}'
		to_json = _to_json_literal(size, f'"{sample}"', f'{sample}L')
		lines = [
			'\t\t// Arrange + Act:',
			f'\t\tfinal {typename} value = new {typename}({sample}L);',
			'',
			'\t\t// Assert: exact value surface',
			f'\t\tassertThat(value.value(), equalTo({sample}L));',
			f'\t\tassertThat(value.size(), equalTo({size}));',
			f'\t\tassertThat(value.isSigned(), equalTo({"false" if ast_model.is_unsigned else "true"}));',
			f'\t\tassertThat(value.toString(), equalTo("{to_string}"));',
			f'\t\tassertThat(value.toJson(), equalTo({to_json}));',
			f'\t\tassertThat(value.serialize(), equalTo({_le_bytes_literal(sample, size)}));',
			'',
			'\t\t// Assert: generated parse arms',
			f'\t\tassertThat({typename}.parse(value), sameInstance(value));',
			f'\t\tassertThat({typename}.parse({sample}L), equalTo(value));',
			f'\t\tassertThat({typename}.parse("{sample}"), equalTo(value));',
		]
		return ModelsSweepTestFormatter._render_method(typename, lines)

	@staticmethod
	def _render_bytes_pod_test(ast_model):
		typename = ast_model.name
		hex_literal = _pattern_hex(ast_model.size, _PATTERN_SEED)
		pattern = f'sweepPatternBytes({typename}.SIZE, {_PATTERN_SEED})'
		lines = [
			'\t\t// Arrange + Act: ascending high-bit byte pattern (byte-order and sign-extension sensitive)',
			f'\t\tfinal {typename} value = new {typename}({pattern});',
			'',
			'\t\t// Assert: exact value surface',
			f'\t\tassertThat(value.size(), equalTo({ast_model.size}));',
			f'\t\tassertThat(value.toString(), equalTo("{hex_literal}"));',
			f'\t\tassertThat(value.toJson(), equalTo("{hex_literal}"));',
			f'\t\tassertThat(value.serialize(), equalTo({pattern}));',
			'',
			'\t\t// Assert: generated parse arms (instance, raw bytes, hex string)',
			f'\t\tassertThat({typename}.parse(value), sameInstance(value));',
			f'\t\tassertThat({typename}.parse({pattern}), equalTo(value));',
			f'\t\tassertThat({typename}.parse("{hex_literal}"), equalTo(value));',
		]
		return ModelsSweepTestFormatter._render_method(typename, lines)

	@staticmethod
	def _render_enum_test(ast_model):
		typename = ast_model.name
		size = ast_model.size
		lines = ['\t\t// Assert: each constant\'s exact surface (values and renderings from the schema)']
		for entry in ast_model.values:
			constant = f'{typename}.{entry.name}'
			value_literal = _int_literal(entry.value, size, ast_model.is_unsigned)
			to_json_literal = _to_json_literal(size, f'"{entry.value}"', value_literal)
			lines.append(f'\t\tassertThat({constant}.getValue(), equalTo({value_literal}));')
			lines.append(f'\t\tassertThat({constant}.toString(), equalTo("{typename}.{entry.name}"));')
			lines.append(f'\t\tassertThat({constant}.toJson(), equalTo({to_json_literal}));')
			lines.append(f'\t\tassertThat({constant}.serialize(), equalTo({_le_bytes_literal(entry.value, size)}));')

		lines.append('')
		lines.append('\t\t// Act + Assert: every constant round-trips through each generated parse arm')
		lines.append(f'\t\tfor (final {typename} constant : {typename}.values()) {{')
		lines.append(f'\t\t\tassertThat(constant.size(), equalTo({size}));')
		lines.append(f'\t\t\tassertThat({typename}.parse(constant), sameInstance(constant));')
		lines.append(f'\t\t\tassertThat({typename}.parse(constant.name().toLowerCase(java.util.Locale.ROOT)), sameInstance(constant));')
		lines.append(f'\t\t\tassertThat({typename}.parse(constant.getValue()), sameInstance(constant));')
		lines.append('\t\t}')
		return ModelsSweepTestFormatter._render_method(typename, lines)

	@staticmethod
	def _render_flags_test(ast_model):
		typename = ast_model.name
		size = ast_model.size
		lines = ['\t\t// Assert: each named constant\'s exact surface and parse arms (values from the schema)']
		for entry in ast_model.values:
			constant = f'{typename}.{entry.name}'
			value_literal = _int_literal(entry.value, size, ast_model.is_unsigned)
			to_json_literal = _to_json_literal(size, f'"{entry.value}"', value_literal)
			lines.append(f'\t\tassertThat({constant}.getValue(), equalTo({value_literal}));')
			lines.append(f'\t\tassertThat({constant}.size(), equalTo({size}));')
			lines.append(f'\t\tassertThat({constant}.toString(), equalTo("{typename}.{entry.name}"));')
			lines.append(f'\t\tassertThat({constant}.toJson(), equalTo({to_json_literal}));')
			lines.append(f'\t\tassertThat({constant}.serialize(), equalTo({_le_bytes_literal(entry.value, size)}));')
			lines.append(f'\t\tassertThat({typename}.parse({constant}), sameInstance({constant}));')
			lines.append(f'\t\tassertThat({typename}.parse("{entry.name.lower()}"), equalTo({constant}));')
			lines.append(f'\t\tassertThat({typename}.parse({constant}.getValue()), equalTo({constant}));')

		return ModelsSweepTestFormatter._render_method(typename, lines)

	@staticmethod
	def _render_method(typename, body_lines):
		body = '\n'.join(body_lines)
		return (
			'\t@Test\n'
			f'\tvoid {uncapitalize(typename)}() {{\n'
			f'{body}\n'
			'\t}\n')

	# -- file rendering -----------------------------------------------------------------------------

	def _render_tests(self):
		tests = []
		for ast_model, formatter in self.entries:
			if DisplayType.STRUCT == ast_model.display_type:
				if ast_model.is_abstract or ast_model.name.endswith(_STRUCT_SKIP_SUFFIX):
					continue

				tests.append(self._render_struct_test(ast_model, formatter))
			elif DisplayType.ENUM == ast_model.display_type:
				tests.append(self._render_flags_test(ast_model) if ast_model.is_bitwise else self._render_enum_test(ast_model))
			elif DisplayType.INTEGER == ast_model.display_type:
				tests.append(self._render_int_pod_test(ast_model))
			elif DisplayType.BYTE_ARRAY == ast_model.display_type:
				tests.append(self._render_bytes_pod_test(ast_model))

		return '\n'.join(tests)

	def render(self):
		return (
			'/**\n'
			' * Generated per-model sweep: every typed property setter/getter pair driven with a distinct value,\n'
			' * cross-checked against the descriptor-pipeline {@code getField}, then re-driven through the\n'
			' * {@code setField} arm with a second distinct value; the sort/size/toString/toJson surface;\n'
			' * best-effort serialize/deserialize; and the exact value surface + parse contract of every pod,\n'
			' * enum and flags constant (expected literals derived from the schema).\n'
			' */\n'
			'final class ModelsSweepTest {\n'
			'\t/** Returns a {@code size}-byte array filled with {@code fill} (distinct per swept field). */\n'
			'\tprivate static byte[] sweepBytes(final int size, final int fill) {\n'
			'\t\tfinal byte[] bytes = new byte[size];\n'
			'\t\tjava.util.Arrays.fill(bytes, (byte) fill);\n'
			'\t\treturn bytes;\n'
			'\t}\n'
			'\n'
			'\t/** Returns a {@code size}-byte ascending pattern starting at {@code seed} (byte-order sensitive). */\n'
			'\tprivate static byte[] sweepPatternBytes(final int size, final int seed) {\n'
			'\t\tfinal byte[] bytes = new byte[size];\n'
			'\t\tfor (int i = 0; i < size; ++i)\n'
			'\t\t\tbytes[i] = (byte) (seed + i);\n'
			'\t\treturn bytes;\n'
			'\t}\n'
			'\n'
			f'{self._render_tests()}'
			'}\n')
