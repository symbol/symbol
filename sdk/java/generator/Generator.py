"""Java catbuffer code-generator entry point. Invoked by ``catparser`` (``-g generator.Generator``)."""

import re
from pathlib import Path

from catparser.DisplayType import DisplayType
from catparser.generators.util import build_factory_map, extend_models

from .EnumTypeFormatter import EnumTypeFormatter
from .FactoryFormatter import FactoryFormatter
from .FlagsTypeFormatter import FlagsTypeFormatter
from .format import separate_if_blocks
from .PodTypeFormatter import PodTypeFormatter
from .printers import BuiltinPrinter, create_pod_printer
from .StructTypeFormatter import StructFormatter
from .TypeFormatter import EnumFormatter, TypeFormatter


def create_printer(descriptor, name, is_pod):
	return (create_pod_printer if is_pod else BuiltinPrinter)(descriptor, name)


def to_type_formatter_instance(ast_model, ast_models, factory_map=None):
	if DisplayType.STRUCT == ast_model.display_type and ast_model.factory_type:
		factory_ast_model = next(candidate for candidate in ast_models if ast_model.factory_type == candidate.name)
		return StructFormatter(ast_model, factory_ast_model, factory_map=factory_map, ast_models=ast_models)

	cls = {
		DisplayType.STRUCT: StructFormatter,
		DisplayType.ENUM: EnumTypeFormatter,
		DisplayType.BYTE_ARRAY: PodTypeFormatter,
		DisplayType.INTEGER: PodTypeFormatter,
	}[ast_model.display_type]
	if DisplayType.ENUM == ast_model.display_type and ast_model.is_bitwise:
		# bitwise flag enums need to be emitted as plain classes (Java enums can't represent
		# OR-combined values), so dispatch to a different formatter that produces a class shape.
		cls = FlagsTypeFormatter

	if cls is StructFormatter:
		# Abstract root structs (no factory_type) — pass factory_map / ast_models so they can
		# emit `permits` and detect NonVerifiable siblings.
		return cls(ast_model, factory_map=factory_map, ast_models=ast_models)

	return cls(ast_model)


def _wrap(formatter, package_name):
	if isinstance(formatter, EnumTypeFormatter):
		return EnumFormatter(formatter, package_name)

	return TypeFormatter(formatter, package_name)


def derive_package_name(output_directory: Path) -> str:
	"""Infer the Java package from the output path (``.../src/main/java/<package>``)."""
	parts = output_directory.parts
	if 'java' in parts:
		# search right-to-left for the source-root 'java' segment
		idx = len(parts) - 1 - list(reversed(parts)).index('java')
		return '.'.join(parts[idx + 1:])

	# fallback for unusual paths
	return 'org.symbol.sdk.generated'


# Fully-qualified-name reference: a package path rooted at java/javax/org (all-lowercase segments) followed
# by the first upper-cased simple type name. ``java.util.Map``, ``org.symbol.sdk.utils.Converter``
_FQN_RE = re.compile(r'((?:java|javax|org)(?:\.[a-z][A-Za-z0-9_]*)+)\.([A-Z][A-Za-z0-9_]*)')


def _split_code_spans(source: str):
	"""Split ``source`` into ``(text, is_code)`` spans, isolating comments and string/char literals. Import resolution must
	ignore fully-qualified names that appear inside comments or string/char literals rather than in real code.
	"""
	spans = []
	index = 0
	length = len(source)
	code_start = 0
	while index < length:
		char = source[index]
		pair = source[index:index + 2]
		if pair == '//':
			spans.append((source[code_start:index], True))
			end = source.find('\n', index)
			end = length if -1 == end else end
			spans.append((source[index:end], False))
			index = code_start = end
		elif pair == '/*':
			spans.append((source[code_start:index], True))
			end = source.find('*/', index + 2)
			end = length if -1 == end else end + 2
			spans.append((source[index:end], False))
			index = code_start = end
		elif char in ('"', "'"):
			spans.append((source[code_start:index], True))
			end = index + 1
			while end < length:
				if source[end] == '\\':
					end += 2
					continue

				if source[end] == char:
					end += 1
					break

				end += 1
			spans.append((source[index:end], False))
			index = code_start = end
		else:
			index += 1

	spans.append((source[code_start:length], True))
	return spans


def finalize_imports(source: str, package_name: str) -> str:
	"""Replace fully-qualified names with imported simple names and emit a sorted import block."""
	# pylint: disable=too-many-locals
	spans = _split_code_spans(source)
	code = ''.join(text for text, is_code in spans if is_code)

	owner_match = re.search(r'\b(?:class|enum|interface)\s+([A-Za-z0-9_]+)', code)
	owner_typename = owner_match.group(1) if owner_match else None

	# Collect candidates; only import a simple name that unambiguously maps to a single FQN.
	candidates = {}
	for match in _FQN_RE.finditer(code):
		simple = match.group(2)
		candidates.setdefault(simple, set()).add(f'{match.group(1)}.{match.group(2)}')

	target_to_simple = {}
	imports = set()
	for simple, targets in candidates.items():
		if 1 != len(targets) or simple == owner_typename:
			continue

		target = next(iter(targets))
		target_to_simple[target] = simple
		imports.add(target)

	if not imports:
		return source

	# Rewrite FQNs to simple names, but only inside code spans.
	replace_re = re.compile(
		'|'.join(re.escape(target) for target in sorted(target_to_simple, key=len, reverse=True)) + r'\b')
	rewritten = []
	for text, is_code in spans:
		rewritten.append(replace_re.sub(lambda m: target_to_simple[m.group(0)], text) if is_code else text)

	source = ''.join(rewritten)

	import_block = '\n'.join(_sort_imports(imports))
	return re.sub(
		rf'(package {re.escape(package_name)};\n)',
		rf'\1\n{import_block}\n',
		source,
		count=1)


def _sort_imports(imports):
	"""Order import statements to match the project's Spotless ``importOrder`` configuration."""
	def group_key(name):
		if name.startswith('java.'):
			return 0

		if name.startswith('javax.'):
			return 1

		if name.startswith('org.bouncycastle'):
			return 2

		if name.startswith('org.symbol'):
			return 4

		return 3

	lines = []
	previous_group = None
	for name in sorted(imports, key=lambda value: (group_key(value), value)):
		group = group_key(name)
		if previous_group is not None and group != previous_group:
			lines.append('')

		lines.append(f'import {name};')
		previous_group = group

	return lines


def generate_files(ast_models, output_directory: Path):
	"""Write one ``.java`` file per type plus a ``Models.java`` manifest; returns the count written."""
	factory_map = build_factory_map(ast_models)
	extend_models(ast_models, create_printer)

	output_directory.mkdir(parents=True, exist_ok=True)
	package_name = derive_package_name(output_directory)

	module_entries = []
	factory_entries = []

	def emit(typename, source):
		source = separate_if_blocks(finalize_imports(source, package_name))
		(output_directory / f'{typename}.java').write_text(source, encoding='utf8')
		module_entries.append(typename)

	for ast_model in ast_models:
		formatter = to_type_formatter_instance(ast_model, ast_models, factory_map=factory_map)
		emit(formatter.typename, str(_wrap(formatter, package_name)))
		# pod/enum/flags types carry a generated `parse(Object)` factory; structs go through setField.
		if isinstance(formatter, (PodTypeFormatter, EnumTypeFormatter, FlagsTypeFormatter)):
			factory_entries.append(formatter.typename)

		# Abstract structs need an additional factory.
		if DisplayType.STRUCT == ast_model.display_type and ast_model.is_abstract:
			factory_formatter = FactoryFormatter(factory_map, ast_model)
			emit(factory_formatter.typename, str(TypeFormatter(factory_formatter, package_name)))

	_write_models_manifest(output_directory, package_name, sorted(module_entries), sorted(factory_entries))

	return len(module_entries)


def _write_models_manifest(output_directory, package_name, module_entries, factory_entries):
	"""Write ``Models.java``: the name→Class MODULE map and the name→parse FACTORIES map."""
	lines = [
		'// Auto-generated by generator. Do not edit directly.',
		'// Run scripts/run_catbuffer_generator.sh to regenerate.',
		'',
		f'package {package_name};',
		'',
		'/**',
		' * Module manifest: every top-level type generated for this blockchain. {@code FACTORIES} maps',
		' * each pod/enum/flags type name to its generated {@code parse} factory.',
		' */',
		'public final class Models {',
		'\tprivate Models() {',
		'\t}',
		'',
		'\t/** Maps every generated type name to its {@link Class}. */',
		'\tpublic static final java.util.Map<String, Class<?>> MODULE;',
		'',
		'\t/** Maps every generated pod/enum/flags type name to its {@code parse} descriptor-value factory. */',
		'\tpublic static final java.util.Map<String, java.util.function.Function<Object, Object>> FACTORIES;',
		'',
		'\tstatic {',
		'\t\tfinal java.util.Map<String, Class<?>> module = new java.util.LinkedHashMap<>();',
		*(f'\t\tmodule.put("{typename}", {typename}.class);' for typename in module_entries),
		'\t\tMODULE = java.util.Collections.unmodifiableMap(module);',
		'',
		'\t\tfinal java.util.Map<String, java.util.function.Function<Object, Object>> factories = new java.util.LinkedHashMap<>();',
		*(f'\t\tfactories.put("{typename}", {typename}::parse);' for typename in factory_entries),
		'\t\tFACTORIES = java.util.Collections.unmodifiableMap(factories);',
		'\t}',
		'}',
		'',
	]
	source = finalize_imports('\n'.join(lines), package_name)
	(output_directory / 'Models.java').write_text(source, encoding='utf8')


class Generator:
	@staticmethod
	def generate(ast_models, output):
		print(f'java catbuffer generator: emitting to {output}')
		count = generate_files(ast_models, Path(output))
		print(f'java catbuffer generator: wrote {count} files')
