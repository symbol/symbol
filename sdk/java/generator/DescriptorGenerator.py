"""Java typed-descriptor code-generator entry point. Invoked by ``catparser`` (``-g generator.DescriptorGenerator``)."""

from pathlib import Path

from catparser.DisplayType import DisplayType
from catparser.generators.util import extend_models

from .DescriptorStructTypeFormatter import DescriptorContext, StructFormatter
from .format import auto_generated_header, separate_if_blocks
from .Generator import create_printer, derive_package_name, finalize_imports

# Model-only structs: fields referencing these are stored verbatim (no nested-descriptor
# unwrap) even though some of them still get their own descriptor class.
_MODEL_ONLY_STRUCTS = frozenset({'NonVerifiableTransaction', 'EmbeddedTransaction', 'Cosignature'})

HEADER = auto_generated_header('run_catbuffer_descriptor_generator.sh')


def _adjusted_display_type(ast_model):
	if ast_model.name in _MODEL_ONLY_STRUCTS:
		return DisplayType.UNSET

	return ast_model.display_type


def _detect_blockchain(output_directory: Path) -> str:
	if 'descriptors' == output_directory.name:
		parent = output_directory.parent.name
		for blockchain in ('nem', 'symbol'):
			if parent == blockchain or parent.startswith(f'{blockchain}_'):
				return blockchain

	return 'symbol'


def _factory_ast_model(ast_model, ast_models):
	if not ast_model.factory_type:
		return None

	return next((candidate for candidate in ast_models if ast_model.factory_type == candidate.name), None)


def _has_descriptor(ast_model, ast_models):
	if DisplayType.STRUCT != ast_model.display_type or ast_model.is_abstract:
		return False

	if not ast_model.factory_type:
		return True

	factory_ast_model = _factory_ast_model(ast_model, ast_models)

	return factory_ast_model is not None and 'Transaction' == factory_ast_model.name


def build_context(ast_models, output_directory: Path) -> DescriptorContext:
	"""Build the descriptor generation context (blockchain + display-type map) for an output path."""
	name_to_display_type_map = {model.name: _adjusted_display_type(model) for model in ast_models}
	return DescriptorContext(_detect_blockchain(output_directory), name_to_display_type_map)


def descriptor_entries(ast_models, context):
	"""``[(ast_model, factory_ast_model, formatter)]`` for every descriptor-bearing struct; shared by the
	descriptor generator and the standalone sweep-test generator so both see the identical set."""
	entries = []
	for ast_model in ast_models:
		if not _has_descriptor(ast_model, ast_models):
			continue

		factory_ast_model = _factory_ast_model(ast_model, ast_models)
		entries.append((ast_model, factory_ast_model, StructFormatter(ast_model, factory_ast_model, context)))

	return entries


def generate_files(ast_models, output_directory: Path):
	"""Write one ``XxxDescriptor.java`` per descriptor-bearing struct plus the sealed family interfaces."""
	extend_models(ast_models, create_printer)

	output_directory.mkdir(parents=True, exist_ok=True)
	package_name = derive_package_name(output_directory)
	context = build_context(ast_models, output_directory)

	written = []
	transaction_names = []
	for _, _, formatter in descriptor_entries(ast_models, context):
		source = f'{HEADER}\npackage {package_name};\n\n{formatter.render()}'
		source = separate_if_blocks(finalize_imports(source, package_name))
		(output_directory / f'{formatter.typename}.java').write_text(source, encoding='utf8')
		written.append(formatter.typename)
		if formatter.base_struct:
			transaction_names.append(formatter.typename)

	if written:
		written += _write_sealed_family(output_directory, context, package_name, written, transaction_names)

	# the per-descriptor sweep test is emitted separately by generator.DescriptorSweepTestGenerator.
	return len(written)


def _write_sealed_family(output_directory, context, package_name, written, transaction_names):
	"""Write the full descriptor-family interface and its transaction-only sub-family; returns their names."""
	sealed_typename = context.sealed_interface
	txn_typename = context.transaction_sealed_interface
	other_names = [name for name in written if name not in transaction_names]

	sealed_doc = [
		f'Sealed {context.blockchain} typed-descriptor family implemented by every generated {{@code XxxDescriptor}} in this',
		'package; extends the cross-blockchain {@link org.symbol.sdk.TypedDescriptor} contract.',
	]
	sealed_source = _render_sealed_interface(
		sealed_typename, 'org.symbol.sdk.TypedDescriptor', [txn_typename] + other_names, sealed_doc, package_name)
	(output_directory / f'{sealed_typename}.java').write_text(sealed_source, encoding='utf8')

	txn_doc = [
		f'Sealed sub-family of {{@link {sealed_typename}}} implemented only by top-level transaction descriptors —',
		'the type accepted by the facade typed-descriptor factory methods.',
	]
	txn_source = _render_sealed_interface(txn_typename, sealed_typename, transaction_names, txn_doc, package_name)
	(output_directory / f'{txn_typename}.java').write_text(txn_source, encoding='utf8')

	return [txn_typename, sealed_typename]


def _render_sealed_interface(typename, extends, permits_names, doc_lines, package_name):
	permits = ',\n\t\t'.join(sorted(permits_names))
	doc = '\n'.join(f' * {line}' for line in doc_lines)
	body = (
		f'{HEADER}\n'
		f'package {package_name};\n'
		'\n'
		'/**\n'
		f'{doc}\n'
		' */\n'
		f'public sealed interface {typename} extends {extends} permits\n'
		f'\t\t{permits} {{\n'
		'}\n'
	)
	return finalize_imports(body, package_name)


class DescriptorGenerator:
	@staticmethod
	def generate(ast_models, output):
		print(f'java catbuffer descriptor generator: emitting to {output}')
		count = generate_files(ast_models, Path(output))
		print(f'java catbuffer descriptor generator: wrote {count} files')
