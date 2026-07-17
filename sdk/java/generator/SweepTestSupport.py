"""Shared plumbing for the standalone sweep-test generators (generator.ModelsSweepTestGenerator and
generator.DescriptorSweepTestGenerator)."""

from pathlib import Path

from .format import separate_if_blocks
from .Generator import finalize_imports


def mirror_test_directory(output_directory: Path) -> Path:
	"""Map a main-tree output directory to its test-tree mirror by rewriting the LAST src/main/java run
	of path components."""
	parts = list(output_directory.parts)
	for index in range(len(parts) - 3, -1, -1):
		if ('src', 'main', 'java') == tuple(parts[index:index + 3]):
			parts[index + 1] = 'test'
			return Path(*parts)

	raise RuntimeError(f'cannot mirror test directory for {output_directory}: no src/main/java segment in path')


def write_sweep_test_file(output_directory, package_name, header, imports, filename, body):
	"""Finalize and write a generated sweep test into the mirrored test-tree package (shared by the
	model and descriptor generators)."""
	# pylint: disable=too-many-arguments,too-many-positional-arguments
	source = f'{header}\npackage {package_name};\n\n{body}'
	source = separate_if_blocks(finalize_imports(source, package_name))
	source = source.replace(f'package {package_name};\n', f'package {package_name};\n\n{imports}', 1)
	test_directory = mirror_test_directory(output_directory)
	test_directory.mkdir(parents=True, exist_ok=True)
	(test_directory / filename).write_text(source, encoding='utf8')
