from collections import namedtuple
from pathlib import Path

import yaml


def load_versions_map(filepath):
	versions = {}
	with open(filepath, 'rt', encoding='utf8') as infile:
		for line in infile.readlines():
			line_parts = line.strip().split(' = ')
			if 2 == len(line_parts):
				versions[line_parts[0]] = line_parts[1]

	return versions


def load_compiler_configuration(filepath):
	with open(filepath, 'rt', encoding='utf8') as configuration_infile:
		configuration_yaml = yaml.load(configuration_infile, Loader=yaml.SafeLoader)
		with open(Path(filepath).parent / configuration_yaml['compiler'], 'rt', encoding='utf8') as compiler_infile:
			compiler_yaml = yaml.load(compiler_infile, Loader=yaml.SafeLoader)

			compiler_keys = ['c', 'cpp', 'version', 'deps']
			compiler = namedtuple('Compiler', compiler_keys)(*[compiler_yaml[key] for key in compiler_keys])

			stl = None
			if 'stl' in compiler_yaml:
				stl_keys = ['version', 'lib']
				stl = namedtuple('Stl', stl_keys)(*[compiler_yaml['stl'][key] for key in stl_keys])

		sanitizers = configuration_yaml['sanitizers'].split(',') if 'sanitizers' in configuration_yaml else []
		architecture = configuration_yaml['architecture']
		enable_code_coverage = configuration_yaml.get('enable_code_coverage', False)

		configuration_keys = ['compiler', 'stl', 'sanitizers', 'architecture', 'enable_code_coverage']
		return namedtuple('CompilerConfiguration', configuration_keys)(compiler, stl, sanitizers, architecture, enable_code_coverage)


def load_build_configuration(filepath):
	with open(filepath, 'rt', encoding='utf8') as configuration_infile:
		configuration_yaml = yaml.load(configuration_infile, Loader=yaml.SafeLoader)

		configuration_keys = ['disposition', 'use_conan', 'enable_diagnostics']
		return namedtuple('BuildConfiguration', configuration_keys)(*[configuration_yaml[key] for key in configuration_keys])
