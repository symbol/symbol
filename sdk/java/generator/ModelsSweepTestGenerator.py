"""Standalone generator for the per-model sweep test. Invoked by catparser
(``-g generator.ModelsSweepTestGenerator``) separately from the model generator: it reconstructs the
model formatters and emits only ``ModelsSweepTest.java`` into the mirrored test tree.
"""

from pathlib import Path

from catparser.generators.util import build_factory_map, extend_models

from .format import auto_generated_header
from .Generator import create_printer, derive_package_name, to_type_formatter_instance
from .ModelsSweepTestFormatter import ModelsSweepTestFormatter
from .SweepTestSupport import write_sweep_test_file

_HEADER = auto_generated_header('run_catbuffer_generator.sh')

# Test-only imports spliced in after FQN finalization (which would otherwise mangle import statements).
_SWEEP_TEST_IMPORTS = (
	'import static org.hamcrest.MatcherAssert.assertThat;\n'
	'import static org.hamcrest.Matchers.equalTo;\n'
	'import static org.hamcrest.Matchers.sameInstance;\n'
	'import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;\n'
	'\n'
	'import org.junit.jupiter.api.Test;\n')


def generate_files(ast_models, output_directory: Path):
	"""Emit ``ModelsSweepTest.java`` into the test tree mirroring ``output_directory``."""
	extend_models(ast_models, create_printer)
	factory_map = build_factory_map(ast_models)
	sweep_entries = [(model, to_type_formatter_instance(model, ast_models, factory_map=factory_map)) for model in ast_models]

	# Always write (dryrun included) into the mirrored test tree — a dryrun run writes to the …_dryrun/models
	# mirror, so scripts/run_catbuffer_generator.sh can diff the sweep test against the committed one and catch
	# content drift, not just emission crashes.
	formatter = ModelsSweepTestFormatter(sweep_entries, ast_models)
	write_sweep_test_file(
		output_directory, derive_package_name(output_directory), header=_HEADER, imports=_SWEEP_TEST_IMPORTS,
		filename='ModelsSweepTest.java', body=formatter.render())
	return 1


class ModelsSweepTestGenerator:
	@staticmethod
	def generate(ast_models, output):
		print(f'java models sweep-test generator: emitting to {output}')
		count = generate_files(ast_models, Path(output))
		print(f'java models sweep-test generator: wrote {count} files')
