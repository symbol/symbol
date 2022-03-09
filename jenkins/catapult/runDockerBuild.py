import argparse
import sys
from pathlib import Path

from BasicBuildManager import BasicBuildManager
from environment import EnvironmentManager
from process import ProcessManager

CCACHE_ROOT = '/jenkins_cache/ccache'
CONAN_ROOT = '/jenkins_cache/conan'

OUTPUT_DIR = Path('output').resolve()
BINARIES_DIR = OUTPUT_DIR / 'binaries'


class OptionsManager(BasicBuildManager):
	def __init__(self, args):
		super().__init__(args.compiler_configuration, args.build_configuration)
		self.operating_system = args.operating_system

	@property
	def image_type(self):
		return 'release' if self.is_release else 'test'

	@property
	def version(self):
		version = self.image_type
		if self.enable_diagnostics:
			version += '-diagnostics'

		return '-'.join([version] + self.sanitizers + [self.architecture])

	@property
	def build_base_image_name(self):
		if self.use_conan:
			name_parts = [self.operating_system, self.versioned_compiler, 'conan']
		else:
			name_parts = [self.operating_system, self.compilation_friendly_name]

		return f'symbolplatform/symbol-server-build-base:{"-".join(name_parts)}'

	@property
	def prepare_base_image_name(self):
		return f'symbolplatform/symbol-server-test-base:{self.operating_system}'

	@property
	def ccache_path(self):
		return Path(CCACHE_ROOT) / ('release' if self.is_release else 'all')

	@property
	def conan_path(self):
		return Path(CONAN_ROOT) / ('clang' if self.is_clang else 'gcc')

	def docker_run_settings(self):
		settings = [
			('CC', self.compiler.c),
			('CXX', self.compiler.cpp),
			('CCACHE_DIR', '/ccache')
		]

		return [f'--env={key}={value}' for key, value in sorted(settings)]


def get_volume_mappings(ccache_path, conan_path, prepare_replacements):
	mappings = [
		(prepare_replacements['source_path'], '/catapult-src'),
		(BINARIES_DIR.resolve(), '/binaries'),
		(conan_path, '/conan'),
		(ccache_path, '/ccache'),
		(prepare_replacements['script_path'], '/scripts')
	]

	return [f'--volume={str(key)}:{value}' for key, value in sorted(mappings)]


def create_docker_run_command(options, prepare_replacements):
	docker_run_settings = options.docker_run_settings()
	volume_mappings = get_volume_mappings(options.ccache_path, options.conan_path, prepare_replacements)

	inner_configuration_path = '/scripts/configurations'
	docker_args = [
		'docker', 'run',
		'--rm',
		f'--user={prepare_replacements["user"]}',
	] + docker_run_settings + volume_mappings + [
		options.build_base_image_name,
		'python3', '/scripts/runDockerBuildInnerBuild.py',
		# assume paths are relative to workdir
		f'--compiler-configuration={inner_configuration_path}/{get_base_from_path(prepare_replacements["compiler_configuration_filepath"])}',
		f'--build-configuration={inner_configuration_path}/{get_base_from_path(prepare_replacements["build_configuration_filepath"])}',
		'--source-path=/catapult-src/client/catapult',
		'--out-dir=/binaries'
	]

	return docker_args


def cleanup_directories(environment_manager, ccache_root_directory, conan_root_directory):
	environment_manager.rmtree(OUTPUT_DIR)
	environment_manager.mkdirs(BINARIES_DIR)

	environment_manager.mkdirs(ccache_root_directory, exist_ok=True)
	environment_manager.mkdirs(conan_root_directory, exist_ok=True)


def prepare_docker_image(process_manager, container_id, prepare_replacements):
	destination_image_label = prepare_replacements['destination_image_label']
	cid_filepath = Path(f'{destination_image_label}.cid')
	if not container_id:
		if cid_filepath.exists():
			cid_filepath.unlink()

	build_disposition = prepare_replacements['build_disposition']
	disposition_to_repository_map = {
		'tests': 'symbol-server-test',
		'private': 'symbol-server-private',
		'public': 'symbol-server'
	}
	destination_repository = disposition_to_repository_map[build_disposition]

	destination_image_name = f'symbolplatform/{destination_repository}:{destination_image_label}'
	script_path = prepare_replacements['script_path']
	process_manager.dispatch_subprocess([
		'docker', 'run',
		f'--cidfile={cid_filepath}',
		f'--volume={script_path}:/scripts',
		f'--volume={OUTPUT_DIR}:/data',
		f'registry.hub.docker.com/{prepare_replacements["base_image_name"]}',
		'python3', '/scripts/runDockerBuildInnerPrepare.py',
		f'--disposition={build_disposition}'
	])

	if not container_id:
		with open(cid_filepath, 'rt', encoding='utf8') as cid_infile:
			container_id = cid_infile.read()

	process_manager.dispatch_subprocess(['docker', 'commit', container_id, destination_image_name])


def get_script_path():
	return Path(sys.argv[0]).resolve().parent


def get_base_from_path(filepath):
	return Path(filepath).name


def main():
	parser = argparse.ArgumentParser(description='catapult project build generator')
	parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
	parser.add_argument('--build-configuration', help='path to build configuration yaml', required=True)
	parser.add_argument('--operating-system', help='operating system', required=True)
	parser.add_argument('--user', help='docker user', required=True)
	parser.add_argument('--destination-image-label', help='docker destination image label', required=True)
	parser.add_argument('--dry-run', help='outputs desired commands without running them', action='store_true')
	parser.add_argument('--base-image-names-only', help='only output the base image names', action='store_true')
	parser.add_argument('--source-path', help='path to the repo root', required=True)
	args = parser.parse_args()

	script_path = Path(get_script_path()).resolve()
	options = OptionsManager(args)

	if args.base_image_names_only:
		print(options.build_base_image_name)
		print(options.prepare_base_image_name)
		return

	source_path = Path(args.source_path).resolve()
	docker_run = create_docker_run_command(options, {
		'source_path': source_path,
		'script_path': script_path,
		'user': args.user,
		'compiler_configuration_filepath': args.compiler_configuration,
		'build_configuration_filepath': args.build_configuration
	})

	environment_manager = EnvironmentManager(args.dry_run)
	environment_manager.rmtree(OUTPUT_DIR)
	environment_manager.mkdirs(BINARIES_DIR)
	environment_manager.mkdirs(options.ccache_path / 'tmp', exist_ok=True)
	environment_manager.mkdirs(options.conan_path, exist_ok=True)

	print('building project')

	process_manager = ProcessManager(args.dry_run)

	return_code = process_manager.dispatch_subprocess(docker_run)
	if return_code:
		sys.exit(return_code)

	print('copying files')

	environment_manager.chdir(OUTPUT_DIR)

	for folder_name in ['scripts', 'seed', 'resources']:
		environment_manager.copy_tree_with_symlinks(source_path / 'client/catapult' / folder_name, folder_name)

	environment_manager.chdir(source_path)

	print('building docker image')

	container_id = '<dry_run_container_id>' if args.dry_run else None
	prepare_docker_image(process_manager, container_id, {
		'base_image_name': options.prepare_base_image_name,
		'destination_image_label': args.destination_image_label,
		'build_disposition': options.build_disposition,
		'source_path': source_path,
		'script_path': script_path
	})


if __name__ == '__main__':
	main()
