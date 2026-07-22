"""Standalone generator for the per-descriptor sweep test."""

from pathlib import Path

from catparser.generators.util import extend_models

from .DescriptorGenerator import HEADER, build_context, descriptor_entries
from .DescriptorSweepTestFormatter import SweepTestFormatter
from .Generator import create_printer, derive_package_name
from .SweepTestSupport import write_sweep_test_file

# Test-only imports spliced in after FQN finalization (which would otherwise mangle import statements).
_SWEEP_TEST_IMPORTS = (
	'import static org.hamcrest.MatcherAssert.assertThat;\n'
	'import static org.hamcrest.Matchers.equalTo;\n'
	'import static org.hamcrest.Matchers.notNullValue;\n'
	'import static org.hamcrest.Matchers.sameInstance;\n'
	'\n'
	'import org.junit.jupiter.api.Test;\n')


def generate_files(ast_models, output_directory: Path):
	"""Emit ``DescriptorsSweepTest.java`` into the test tree mirroring ``output_directory``."""
	extend_models(ast_models, create_printer)
	context = build_context(ast_models, output_directory)
	entries = descriptor_entries(ast_models, context)
	if not entries:
		return 0

	# Always write (dryrun included) into the mirrored test tree — a dryrun writes to the
	# …_descriptor_dryrun/descriptors mirror so scripts/run_catbuffer_descriptor_generator.sh can diff the
	# sweep test against the committed one and catch content drift, not just emission crashes.
	formatter = SweepTestFormatter(entries, context, ast_models)
	write_sweep_test_file(
		output_directory, derive_package_name(output_directory), header=HEADER, imports=_SWEEP_TEST_IMPORTS,
		filename='DescriptorsSweepTest.java', body=formatter.render())
	return 1


class DescriptorSweepTestGenerator:
	@staticmethod
	def generate(ast_models, output):
		print(f'java descriptor sweep-test generator: emitting to {output}')
		count = generate_files(ast_models, Path(output))
		print(f'java descriptor sweep-test generator: wrote {count} files')
