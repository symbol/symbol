import argparse
import os
import shutil
from pathlib import Path

from BasicBuildManager import BasicBuildManager
from environment import EnvironmentManager
from process import ProcessManager

CONAN_NEMTECH_REMOTE = 'https://conan.symbol.dev/artifactory/api/conan/catapult'


class BuildEnvironment:
	def __init__(self, use_conan, process_manager, environment_manager):
		self.use_conan = use_conan
		self.dispatch_subprocess = process_manager.dispatch_subprocess
		self.environment_manager = environment_manager

	def prepare(self, build_path):
		self._prepare_directory(build_path)
		self._prepare_environment_variables()
		self.dispatch_subprocess(['ccache', '-M', '30G'])
		self.dispatch_subprocess(['ccache', '-s'])

	def _prepare_directory(self, build_path):
		self.environment_manager.mkdirs(build_path)
		self.environment_manager.chdir(build_path)

	def _prepare_environment_variables(self):
		if self.use_conan:
			# conan cache directory
			self.environment_manager.set_env_var('CONAN_HOME', '/conan')
		else:
			if self.environment_manager.is_windows_platform():
				self.environment_manager.set_env_var('BOOST_ROOT', 'c:/usr/catapult/deps/boost')
				self.environment_manager.set_env_var('GTEST_ROOT', 'c:/usr/catapult/deps/google')
			else:
				self.environment_manager.set_env_var('BOOST_ROOT', '/mybuild')
				self.environment_manager.set_env_var('GTEST_ROOT', '/usr/local')

	def prepare_conan(self):
		# create default profile if it does not exist
		if self.dispatch_subprocess(['conan', 'profile', 'show', '--profile', 'default'], show_output=False, handle_error=False):
			self.dispatch_subprocess(['conan', 'profile', 'detect', '--name', 'default'])

		self.dispatch_subprocess(['conan', 'profile', 'show', '--profile', 'default'])
		self.dispatch_subprocess(['conan', 'remote', 'add', '--force', 'nemtech', CONAN_NEMTECH_REMOTE])

	def run_conan_install(self, source_path, settings, build_path, build_type):
		# assuming working directory == build directory
		setting_overrides = []
		for key, value in settings.items():
			setting_overrides += ['-s', f'compiler.{key}={value}']
		self.dispatch_subprocess(['conan', 'profile', 'show', '--profile', 'default'])
		self.dispatch_subprocess([
			'conan', 'install', source_path,
			'--build', 'missing',
			'--output-folder', build_path,
			'-s', f'build_type={build_type}',
		] + setting_overrides)


class BuildManager(BasicBuildManager):
	def __init__(self, args, process_manager, environment_manager, build_type):
		super().__init__(args.compiler_configuration, args.build_configuration)
		self.dispatch_subprocess = process_manager.dispatch_subprocess
		self.list_dir = process_manager.list_dir
		self.environment_manager = environment_manager
		self.build_type = build_type

	def cmake_settings(self, output_path):
		settings = [
			('CMAKE_INSTALL_PREFIX', output_path),
			('CMAKE_BUILD_TYPE', self.build_configuration),
			('CATAPULT_TEST_DB_URL', 'mongodb://db:27017'),
			('CATAPULT_DOCKER_TESTS', 'ON'),
			('ENABLE_CODE_COVERAGE', 'ON' if self.enable_code_coverage else 'OFF')
		]

		if self.environment_manager.is_windows_platform():
			settings.append(('USE_CCACHE_ON_WINDOWS', 'ON'))
		else:
			if 'arm64' != self.architecture:
				# ARCHITECTURE_NAME is used to set `-march`, disable on windows and arm
				settings.append(('ARCHITECTURE_NAME', self.architecture))

		if self.enable_diagnostics:
			settings.append(('ENABLE_CATAPULT_DIAGNOSTICS', 'ON'))

		if self.use_conan:
			settings.append(('USE_CONAN', 'ON'))
		else:
			if self.environment_manager.is_windows_platform():
				settings.append((
					'CMAKE_PREFIX_PATH',
					';'.join(f'c:/usr/catapult/deps/{package}' for package in ['boost', 'facebook', 'google', 'mongodb', 'openssl', 'zeromq'])
				))
			else:
				settings.append(('OPENSSL_ROOT_DIR', '/usr/catapult/deps'))

		if self.sanitizers:
			settings.append(('USE_SANITIZER', ','.join(self.sanitizers)))

		if self.is_release:
			settings.append(('CATAPULT_BUILD_RELEASE', 'ON'))
			settings.append(('ENABLE_TESTS', 'OFF'))

		if 'public' == self.build_disposition:
			settings.append(('CATAPULT_BUILD_RELEASE_PUBLIC', 'ON'))

		return [f'-D{key}={value}' for key, value in settings]

	def run_cmake(self, source_path, output_path, cmake_preset):
		cmake_settings = self.cmake_settings(output_path)
		if self.environment_manager.is_windows_platform():
			self.dispatch_subprocess(
				['cmake'] + cmake_preset + cmake_settings + [
					'-G', 'Visual Studio 16 2019' if 16 == self.compiler.version else 'Visual Studio 17 2022', '-A', 'x64', source_path
				]
			)
		else:
			self.dispatch_subprocess(['cmake'] + cmake_preset + cmake_settings + ['-G', 'Ninja', source_path])

	def build(self):
		if self.environment_manager.is_windows_platform():
			# copy the real ccache.exe since shim version is in the path
			shutil.copy2('C:/Users/ContainerAdministrator/scoop/apps/ccache/current/ccache.exe', 'c:/tmp/_build/cl.exe')
			self.dispatch_subprocess(['cmake', '--build', '.', '--target', 'publish'])
			self.dispatch_subprocess([
				'msbuild',
				f'/p:Configuration={self.build_type}',
				'/p:Platform=x64',
				'/m',
				'/p:UseMultiToolTask=true',
				'INSTALL.vcxproj',
				'/p:CLToolPath=c:/tmp/_build'
			],
				True,
				False
			)
		else:
			cpu_count = os.cpu_count()
			cpu_count_str = str(cpu_count if cpu_count > 0 else 1)
			self.dispatch_subprocess(['ninja', 'publish'])
			self.dispatch_subprocess(['ninja', '-j', cpu_count_str])
			self.dispatch_subprocess(['ninja', 'install'])

		self.dispatch_subprocess(['ccache', '--show-stats'])

	def copy_dependencies(self, destination):
		if self.use_conan:
			return

		self.environment_manager.mkdirs(destination)
		if self.environment_manager.is_windows_platform():
			self.environment_manager.copy_glob_with_symlinks('c:/usr/catapult/deps/boost/lib', '*.dll', destination)
			for name in ['facebook', 'mongodb', 'openssl', 'zeromq']:
				self.environment_manager.copy_glob_with_symlinks(f'c:/usr/catapult/deps/{name}/bin', '*.dll', destination)

			for name in ['engines-3', 'ossl-modules']:
				self.environment_manager.copy_tree_with_symlinks(f'c:/usr/catapult/deps/openssl/lib/{name}', Path(destination) / 'openssl/lib' / name)

			return

		for name in ['atomic', 'chrono', 'date_time', 'filesystem', 'log', 'log_setup', 'program_options', 'regex', 'thread']:
			self.environment_manager.copy_glob_with_symlinks('/mybuild/lib', f'libboost_{name}.so*', destination)

		for name in ['bson-1.0', 'mongoc-1.0', 'bsoncxx', 'mongocxx', 'zmq', 'rocksdb', 'snappy', 'gflags']:
			system_bin_path = self.environment_manager.system_bin_path
			self.environment_manager.copy_glob_with_symlinks(system_bin_path, f'lib{name}.so*', destination)

		openssl_source_directory = Path('/usr/catapult/deps')
		self.environment_manager.mkdirs(Path(destination), exist_ok=True)
		for name in ['crypto', 'ssl']:
			self.environment_manager.copy_glob_with_symlinks(openssl_source_directory, f'lib{name}.so*', Path(destination))

		for name in ['engines-3', 'ossl-modules']:
			self.environment_manager.copy_tree_with_symlinks(openssl_source_directory / name, Path(destination) / name)

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

		self.copy_dependencies(deps_output_path)
		self.copy_compiler_deps(deps_output_path)

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
	parser.add_argument('--build-type', help='build type for the build', default='RelWithDebInfo')
	args = parser.parse_args()

	process_manager = ProcessManager(args.dry_run)
	environment_manager = EnvironmentManager(args.dry_run)

	builder = BuildManager(args, process_manager, environment_manager, args.build_type)
	conan_options = {'version': builder.compiler.version, 'libcxx': builder.stl.lib}
	if builder.is_msvc:
		conan_options = {'cppstd': 17}

	env = BuildEnvironment(builder.use_conan, process_manager, environment_manager)
	build_path = f'{args.source_path}/_build' if builder.enable_code_coverage else '/tmp/_build'
	env.prepare(build_path)

	cmake_preset = []
	if builder.use_conan:
		env.prepare_conan()
		env.run_conan_install(args.source_path, conan_options, build_path, args.build_type)
		environment_manager.chdir(f'{build_path}/build' if environment_manager.is_windows_platform() else f'{build_path}/build/{args.build_type}')
		conan_preset_name = 'conan-default' if environment_manager.is_windows_platform() else f'conan-{args.build_type.lower()}'
		cmake_preset = [f'--preset={conan_preset_name}']

	builder.run_cmake(args.source_path, args.out_dir, cmake_preset)
	builder.build()
	builder.copy_files(args.out_dir)


if __name__ == '__main__':
	main()
