import argparse
import re
import subprocess
from pathlib import Path

from conan.tools.scm import Version
from configuration import load_versions_map

CONAN_NEMTECH_REMOTE = 'https://conan.symbol.dev/artifactory/api/conan/catapult'
NEMTECH_REMOTE_NAME = 'nemtech'

# dependency name(conan) -> version property and cmake name
DEPENDENCY_NAMES_MAP = {
	'boost': ('boost', 'Boost'),
	'benchmark': ('google_benchmark', 'benchmark'),
	'gtest': ('google_googletest', 'GTest'),
	'mongo-c-driver': ('mongodb_mongo-c-driver', 'MONGOC-1.0'),
	'mongo-cxx-driver': ('mongodb_mongo-cxx-driver', 'MONGOCXX'),
	'zeromq': ('zeromq_libzmq', None),
	'cppzmq': ('zeromq_cppzmq', 'cppzmq'),
	'rocksdb': ('facebook_rocksdb', 'RocksDB'),
	'openssl': ('openssl_openssl', 'OpenSSL')
}
DEPENDENCIES_NAME = DEPENDENCY_NAMES_MAP.keys()


def dispatch_subprocess(command_line, cwd=None, handle_error=True):
	print(' '.join(command_line))
	result = subprocess.run(command_line, check=False, cwd=cwd, capture_output=True)
	if handle_error and 0 != result.returncode:
		raise subprocess.SubprocessError(f'command failed with exit code {result.returncode}\n{result}')

	output = result.stdout.decode('utf-8') if 0 == result.returncode else result.stderr.decode('utf-8')
	if output:
		print(output)

	return output, result.returncode


class CatapultDependencyUpdater:
	def __init__(self, versions_file, source_path):
		self.versions_file = Path(versions_file).absolute()
		self.source_path = Path(source_path).absolute()
		self.versions = load_versions_map(versions_file)
		self._setup_conan_environment()

	@staticmethod
	def _setup_conan_environment():
		_, error_code = dispatch_subprocess(['conan', 'profile', 'show', '--profile', 'default'], handle_error=False)
		if 0 != error_code:
			dispatch_subprocess(['conan', 'profile', 'detect', '--name', 'default'])

		dispatch_subprocess(['conan', 'profile', 'show', '--profile', 'default'])
		dispatch_subprocess(['conan', 'remote', 'add', '--force', NEMTECH_REMOTE_NAME, CONAN_NEMTECH_REMOTE])

	@staticmethod
	def _is_remote_nemtech(dependency_name):
		return dependency_name in ['benchmark', 'cppzmq', 'zeromq', 'mongo-c-driver', 'mongo-cxx-driver', 'rocksdb']

	def _get_conan_latest_version(self, name):
		query = f'{name}/*' if not self._is_remote_nemtech(name) else f'{name}/*@{NEMTECH_REMOTE_NAME}/stable'
		remote_name = NEMTECH_REMOTE_NAME if self._is_remote_nemtech(name) else 'conancenter'
		output, _ = dispatch_subprocess(['conan', 'search', query, '--remote', remote_name])
		version_list = [line.split('/')[1].split('@')[0] for line in output.splitlines() if '/' in line]
		filtered_versions = [version for version in version_list if version[0].isdigit()]
		return filtered_versions[-1]

	def get_available_update(self, dependencies):
		dependencies_with_updates = []
		for dependency in dependencies:
			latest_version = self._get_conan_latest_version(dependency)
			current_version = re.sub(r'[^\d\.]', '', self.versions[DEPENDENCY_NAMES_MAP[dependency][0]])
			print(f'checking dependency {dependency} {current_version} -> {latest_version}')
			if Version(latest_version) > Version(current_version):
				print(f'{dependency} {current_version} -> {latest_version}')
				dependencies_with_updates.append((dependency, current_version, latest_version))

		return dependencies_with_updates

	def update_dependencies(self, dependencies_to_update):
		def update_version_in_file(name, filepath, old_version, new_version, get_current_version):
			current_name_version = get_current_version(name, old_version)
			new_name_version = current_name_version.replace(old_version, new_version)
			print(f'updating dependency {current_name_version} -> {new_name_version}')
			dispatch_subprocess(['sed', '-i', f's|{current_name_version}|{new_name_version}|g', str(filepath)])

		dependency_files = {
			'boost': ['CMakeLists.txt'],
			'benchmark': ['tests/bench/CMakeLists.txt'],
			'gtest': ['CMakeLists.txt'],
			'mongo-c-driver': ['extensions/mongo/CMakeLists.txt'],
			'mongo-cxx-driver': ['extensions/mongo/CMakeLists.txt'],
			'cppzmq': ['extensions/zeromq/CMakeLists.txt'],
			'openssl': ['CMakeLists.txt'],
			'rocksdb': ['CMakeLists.txt'],
		}

		for dependency_name, current_version, latest_version in dependencies_to_update:
			property_name, cmake_name = DEPENDENCY_NAMES_MAP[dependency_name]
			print(f'updating dependency {dependency_name} from {current_version} -> {latest_version}')
			update_version_in_file(
				property_name,
				self.versions_file,
				current_version,
				latest_version,
				lambda name, version: f'{name} = {self.versions[property_name]}'  # pylint: disable=cell-var-from-loop
			)

			update_version_in_file(
				dependency_name,
				self.source_path.joinpath('conanfile.py'),
				current_version,
				latest_version,
				lambda name, version: f'{name}/{version}'
			)

			for cmake_file in dependency_files.get(dependency_name, []):
				update_version_in_file(
					cmake_name,
					self.source_path.joinpath(cmake_file),
					current_version,
					latest_version,
					lambda name, version: f'{name} {version}'
				)


def main():
	parser = argparse.ArgumentParser(description='Update catapult dependencies')
	parser.add_argument('--dependencies', choices=DEPENDENCIES_NAME, nargs='*', help='dependencies to update', default=DEPENDENCIES_NAME)
	parser.add_argument('--versions-file', help='path to the versions file', required=True)
	parser.add_argument('--source-path', help='path to the catapult source', required=True)
	parser.add_argument('--commit', help='commit changes to git', action='store_false')
	args = parser.parse_args()

	dependency_updater = CatapultDependencyUpdater(args.versions_file, args.source_path)
	dependencies_to_update = dependency_updater.get_available_update(args.dependencies)
	print(f'dependencies to update: {dependencies_to_update}')
	if not dependencies_to_update:
		print('no dependencies to update')
		return

	dependency_updater.update_dependencies(dependencies_to_update)
	update_message = '\n'.join([f'{dependency[0]} {dependency[1]} -> {dependency[2]}' for dependency in dependencies_to_update])
	print(f'updated dependencies:\n{update_message}')

	if args.commit:
		dispatch_subprocess(['git', 'add', '.'])
		dispatch_subprocess(['git', 'commit', '-m', f'[client/catapult] fix: update catapult dependency\n\n{update_message}'])


if '__main__' == __name__:
	main()
