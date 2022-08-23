import argparse
import os
from pathlib import Path

from BasicBuildManager import BasicBuildManager
from environment import EnvironmentManager
from process import ProcessManager

CONAN_NEMTECH_REMOTE = 'https://catapult.jfrog.io/artifactory/api/conan/symbol-conan'


class BuildEnvironment:
	def __init__(self, use_conan, process_manager, environment_manager):
		self.use_conan = use_conan
		self.dispatch_subprocess = process_manager.dispatch_subprocess
		self.environment_manager = environment_manager

	def prepare(self, enable_code_coverage, out_dir):
		self._prepare_directory(enable_code_coverage, out_dir)
		self._prepare_environment_variables()

		if not self.environment_manager.is_windows_platform():
			self.dispatch_subprocess(['ccache', '-M', '30G'])
			self.dispatch_subprocess(['ccache', '-s'])

	def _prepare_directory(self, enable_code_coverage, out_dir):
		build_path = f'{out_dir}/_build' if enable_code_coverage else '/tmp/_build'
		self.environment_manager.mkdirs(build_path)
		self.environment_manager.chdir(build_path)

	def _prepare_environment_variables(self):
		if self.use_conan:
			# conan cache directory
			if self.environment_manager.is_windows_platform():
				self.environment_manager.set_env_var('CONAN_USER_HOME', '/conan')
				self.environment_manager.set_env_var('CONAN_USER_HOME_SHORT', 'None')
			else:
				self.environment_manager.set_env_var('HOME', '/conan')
		else:
			if self.environment_manager.is_windows_platform():
				self.environment_manager.set_env_var('BOOST_ROOT', 'c:/deps/boost')
				self.environment_manager.set_env_var('GTEST_ROOT', 'c:/deps/google')
			else:
				self.environment_manager.set_env_var('BOOST_ROOT', '/mybuild')
				self.environment_manager.set_env_var('GTEST_ROOT', '/usr/local')

	def prepare_conan(self, settings):
		# create default profile if it does not exist
		if self.dispatch_subprocess(['conan', 'profile', 'get', 'settings.compiler', 'default'], show_output=False, handle_error=False):
			self.dispatch_subprocess(['conan', 'profile', 'new', 'default', '--detect'])

		self.dispatch_subprocess(['conan', 'profile', 'show', 'default'])
		self.dispatch_subprocess(['conan', 'remote', 'add', '--force', 'nemtech', CONAN_NEMTECH_REMOTE])
		self.dispatch_subprocess(['conan', 'config', 'set', 'general.revisions_enabled=True'])

		for key, value in settings.items():
			self.dispatch_subprocess(['conan', 'profile', 'update', f'settings.compiler.{key}={value}', 'default'])

	def run_conan_install(self, source_path):
		# assuming working directory == build directory
		self.dispatch_subprocess(['conan', 'profile', 'show', 'default'])
		self.dispatch_subprocess(['conan', 'install', source_path, '--build', 'missing'])


class BuildManager(BasicBuildManager):
	def __init__(self, args, process_manager, environment_manager):
		super().__init__(args.compiler_configuration, args.build_configuration)
		self.dispatch_subprocess = process_manager.dispatch_subprocess
		self.list_dir = process_manager.list_dir
		self.environment_manager = environment_manager

	def cmake_settings(self, output_path):
		settings = [
			('CMAKE_INSTALL_PREFIX', output_path),
			('CMAKE_BUILD_TYPE', self.build_configuration),
			('CATAPULT_TEST_DB_URL', 'mongodb://db:27017'),
			('CATAPULT_DOCKER_TESTS', 'ON'),
			('ENABLE_CODE_COVERAGE', 'ON' if self.enable_code_coverage else 'OFF')
		]

		if not self.environment_manager.is_windows_platform():
			# ARCHITECTURE_NAME is used to set `-march`, disable on windows
			settings.append(('ARCHITECTURE_NAME', self.architecture))

		if self.enable_diagnostics:
			settings.append(('ENABLE_CATAPULT_DIAGNOSTICS', 'ON'))

		if self.use_conan:
			settings.append(('USE_CONAN', 'ON'))
		else:
			if self.environment_manager.is_windows_platform():
				settings.append((
					'CMAKE_PREFIX_PATH',
					';'.join(f'c:/deps/{package}' for package in ['boost', 'facebook', 'google', 'mongodb', 'openssl', 'zeromq'])
				))

		if self.sanitizers:
			settings.extend([
				('USE_SANITIZER', ','.join(self.sanitizers)),
				('OPENSSL_ROOT_DIR', '/usr/local')
			])

		if self.is_release:
			settings.append(('CATAPULT_BUILD_RELEASE', 'ON'))
			settings.append(('ENABLE_TESTS', 'OFF'))

		if 'public' == self.build_disposition:
			settings.append(('CATAPULT_BUILD_RELEASE_PUBLIC', 'ON'))

		return [f'-D{key}={value}' for key, value in settings]

	def run_cmake(self, source_path, output_path):
		cmake_settings = self.cmake_settings(output_path)
		if self.environment_manager.is_windows_platform():
			self.dispatch_subprocess(
				['cmake'] + cmake_settings + [
					'-G', 'Visual Studio 16 2019' if 16 == self.compiler.version else 'Visual Studio 17 2022', '-A', 'x64', source_path
				]
			)
		else:
			self.dispatch_subprocess(['cmake'] + cmake_settings + ['-G', 'Ninja', source_path])

	def build(self):
		if self.environment_manager.is_windows_platform():
			self.dispatch_subprocess(['cmake', '--build', '.', '--target', 'publish'])
			self.dispatch_subprocess(
				['msbuild', '/p:Configuration=RelWithDebInfo', '/p:Platform=x64', '/m', 'ALL_BUILD.vcxproj'],
				True,
				False
			)
			self.dispatch_subprocess(
				['msbuild', '/p:Configuration=RelWithDebInfo', '/p:Platform=x64', '/m', 'INSTALL.vcxproj'],
				True,
				False
			)
		else:
			cpu_count = os.cpu_count()
			cpu_count_str = str(cpu_count if cpu_count > 0 else 1)
			self.dispatch_subprocess(['ninja', 'publish'])
			self.dispatch_subprocess(['ninja', '-j', cpu_count_str])
			self.dispatch_subprocess(['ninja', 'install'])

	def copy_dependencies(self, destination):
		if self.use_conan:
			if not self.environment_manager.is_windows_platform():
				self.environment_manager.copy_tree_with_symlinks('./deps', destination)
			return

		self.environment_manager.mkdirs(destination)
		if self.environment_manager.is_windows_platform():
			self.environment_manager.copy_glob_with_symlinks('c:/deps/boost/lib', '*.dll', destination)
			for name in ['facebook', 'mongodb', 'openssl', 'zeromq']:
				self.environment_manager.copy_glob_with_symlinks(f'c:/deps/{name}/bin', '*.dll', destination)

			self.environment_manager.copy_tree_with_symlinks('c:/deps/openssl/lib/engines-1_1', Path(destination) / 'engines-1_1')
			return

		for name in ['atomic', 'chrono', 'date_time', 'filesystem', 'log', 'log_setup', 'program_options', 'regex', 'thread']:
			self.environment_manager.copy_glob_with_symlinks('/mybuild/lib', f'libboost_{name}.so*', destination)

		for name in ['bson-1.0', 'mongoc-1.0', 'bsoncxx', 'mongocxx', 'zmq', 'rocksdb', 'snappy', 'gflags']:
			system_bin_path = self.environment_manager.system_bin_path
			self.environment_manager.copy_glob_with_symlinks(system_bin_path, f'lib{name}.so*', destination)

		local_lib_path = self.environment_manager.local_lib_path
		for name in ['crypto', 'ssl']:
			self.environment_manager.copy_glob_with_symlinks(local_lib_path, f'lib{name}*', destination)

		self.environment_manager.copy_tree_with_symlinks(f'{local_lib_path}/engines-1.1', Path(destination) / 'engines-1.1')

	def copy_compiler_deps(self, destination):
		if not self.compiler.deps:
			return

		for dependency_pattern in self.compiler.deps:
			directory_path = os.path.dirname(dependency_pattern)
			pattern = os.path.basename(dependency_pattern)
			self.environment_manager.copy_glob_with_symlinks(directory_path, pattern, destination)

	def copy_files(self, output_path):
		deps_output_path = Path(f'{output_path}/deps').resolve()
		tests_output_path = Path(f'{output_path}/tests').resolve()

		# copy deps into the tests folder for windows
		dest_path = tests_output_path if EnvironmentManager.is_windows_platform() else deps_output_path
		self.copy_dependencies(dest_path)
		self.copy_compiler_deps(dest_path)

		# copy tests
		if not self.is_release:
			self.environment_manager.mkdirs(tests_output_path, exist_ok=True)
			self.environment_manager.copy_glob_subtree_with_symlinks('./bin', 'tests*', tests_output_path)
			if EnvironmentManager.is_windows_platform():
				self.environment_manager.copy_glob_subtree_with_symlinks('./bin', '*.dll', tests_output_path)

		# list directories
		self.list_dir(output_path)
		self.list_dir(f'{output_path}/lib')
		if not EnvironmentManager.is_windows_platform():
			self.list_dir(deps_output_path)

		if not self.is_release:
			self.list_dir(tests_output_path)


def main():
	parser = argparse.ArgumentParser(description='catapult project build generator')
	parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
	parser.add_argument('--build-configuration', help='path to build configuration yaml', required=True)
	parser.add_argument('--dry-run', help='outputs desired commands without running them', action='store_true')
	parser.add_argument('--source-path', help='path to the catapult source code', required=True)
	parser.add_argument('--out-dir', help='output path to copy catapult binaries', required=True)
	args = parser.parse_args()

	process_manager = ProcessManager(args.dry_run)
	environment_manager = EnvironmentManager(args.dry_run)

	builder = BuildManager(args, process_manager, environment_manager)
	conan_options = {'version': builder.compiler.version, 'libcxx': builder.stl.lib}
	if builder.is_msvc:
		conan_options = {'version': builder.compiler.version}

	env = BuildEnvironment(builder.use_conan, process_manager, environment_manager)
	env.prepare(builder.enable_code_coverage, args.source_path)

	if builder.use_conan:
		env.prepare_conan(conan_options)
		env.run_conan_install(args.source_path)

	builder.run_cmake(args.source_path, args.out_dir)
	builder.build()
	builder.copy_files(args.out_dir)


if __name__ == '__main__':
	main()
