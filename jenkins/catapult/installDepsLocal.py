import argparse
from pathlib import Path

from configuration import load_versions_map
from dependency_flags import get_dependency_flags
from environment import EnvironmentManager
from process import ProcessManager

SOURCE_DIR_NAME = 'source'
NUM_BUILD_CORES = 8


class Downloader:
	def __init__(self, versions, process_manager):
		self.versions = versions
		self.process_manager = process_manager

	def download_boost(self):
		if EnvironmentManager.is_windows_platform():
			self.download_boost_windows()
		else:
			self.download_boost_unix()

	def download_boost_unix(self):
		version = self.versions['boost']
		tar_filename = f'boost_1_{version}_0.tar.gz'
		tar_source_path = f'https://boostorg.jfrog.io/artifactory/main/release/1.{version}.0/source/{tar_filename}'

		self.process_manager.dispatch_subprocess(['curl', '-o', tar_filename, '-SL', tar_source_path])
		self.process_manager.dispatch_subprocess(['tar', '-xzf', tar_filename])
		self.process_manager.dispatch_subprocess(['mv', f'boost_1_{version}_0', 'boost'])

	def download_boost_windows(self):
		version = self.versions['boost']
		archive_name = f'boost_1_{version}_0'
		zip_filename = f'{archive_name}.7z'
		zip_source_path = f'https://boostorg.jfrog.io/artifactory/main/release/1.{version}.0/source/{zip_filename}'

		self.process_manager.dispatch_subprocess(['powershell', '-Command', 'wget', zip_source_path, '-outfile', zip_filename])
		self.process_manager.dispatch_subprocess(['powershell', '-Command', '7z', 'x', zip_filename])
		self.process_manager.dispatch_subprocess(['powershell', '-Command', 'dir'])
		self.process_manager.dispatch_subprocess(['powershell', '-Command', 'Move-Item', rf'{archive_name}', 'boost'])

	def download_git_dependency(self, organization, project):
		version = self.versions[f'{organization}_{project}']
		repository = f'https://github.com/{organization}/{project}.git'
		self.process_manager.dispatch_subprocess(['git', 'clone', '-b', version, repository])


class Builder:
	def __init__(self, target_directory, versions, process_manager, environment_manager):
		self.target_directory = Path(target_directory)
		self.versions = versions
		self.process_manager = process_manager
		self.environment_manager = environment_manager
		self.is_clang = False

	def use_clang(self):
		self.is_clang = True

	def build_boost(self):
		self.environment_manager.chdir(self.target_directory / SOURCE_DIR_NAME / 'boost')

		boost_prefix_option = f'--prefix={self.target_directory / "boost"}'
		bootstrap_options = [r'.\bootstrap.bat' if EnvironmentManager.is_windows_platform() else './bootstrap.sh']
		if self.is_clang:
			bootstrap_options += ['with-toolset=clang']

		self.process_manager.dispatch_subprocess(bootstrap_options)

		b2_options = [boost_prefix_option]
		if self.is_clang:
			b2_options += ['toolset=clang', 'linkflags=\'-stdlib=libc++\'']

		b2_options += get_dependency_flags('boost')

		b2_filepath = r'.\b2' if EnvironmentManager.is_windows_platform() else './b2'
		self.process_manager.dispatch_subprocess([b2_filepath] + b2_options + ['-j', str(NUM_BUILD_CORES), 'stage', 'release'])
		self.process_manager.dispatch_subprocess([b2_filepath, 'install'] + b2_options)

	def build_git_dependency(self, organization, project):
		self.environment_manager.chdir(self.target_directory / SOURCE_DIR_NAME / project)
		self.environment_manager.mkdirs('_build', exist_ok=True)
		self.environment_manager.chdir('_build')

		cmake_options = [
			'cmake', '-DCMAKE_BUILD_TYPE=RelWithDebInfo', f'-DCMAKE_INSTALL_PREFIX={self.target_directory / organization}'
		]

		if self.is_clang:
			cmake_options += ['-DCMAKE_CXX_COMPILER=\'clang++\'', '-DCMAKE_CXX_FLAGS=\'-std=c++1y -stdlib=libc++\'']

		if EnvironmentManager.is_windows_platform() and 'mongo-cxx-driver' == project:
			# For build without a C++17 polyfill
			# https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
			cmake_options += ['-DCMAKE_CXX_FLAGS="/Zc:__cplusplus"', f'-DCMAKE_PREFIX_PATH={self.target_directory / organization}']

		if 'mongodb' == organization:
			cmake_options += [f'-DOPENSSL_ROOT_DIR={self.target_directory / "openssl"}']

		additional_cmake_options = get_dependency_flags(f'{organization}_{project}')
		if additional_cmake_options:
			cmake_options += additional_cmake_options

		self.process_manager.dispatch_subprocess(cmake_options + ['..'])
		if EnvironmentManager.is_windows_platform():
			self.process_manager.dispatch_subprocess(['cmake', '--build', '.', '-j', str(NUM_BUILD_CORES), '--target', 'install'])
		else:
			self.process_manager.dispatch_subprocess(['make', '-j', str(NUM_BUILD_CORES)])
			self.process_manager.dispatch_subprocess(['make', 'install'])

	def build_openssl(self):
		self.environment_manager.chdir(self.target_directory / SOURCE_DIR_NAME / 'openssl')

		if EnvironmentManager.is_windows_platform():
			self.build_openssl_windows()
		else:
			self.build_openssl_unix()

	def build_openssl_windows(self):
		openssl_destinations = [f'--{key}={self.target_directory / "openssl"}' for key in ('prefix', 'openssldir')]
		self.process_manager.dispatch_subprocess(['perl', './Configure', 'VC-WIN64A'] + openssl_destinations)
		self.process_manager.dispatch_subprocess(['nmake'])
		self.process_manager.dispatch_subprocess(['nmake', 'install_sw', 'install_ssldirs'])

	def build_openssl_unix(self):
		compiler = 'linux-x86_64-clang' if self.is_clang else ''
		openssl_destinations = [f'--{key}={self.target_directory / "openssl"}' for key in ('prefix', 'openssldir', 'libdir')]
		self.process_manager.dispatch_subprocess(['perl', './Configure', compiler] + openssl_destinations)
		self.process_manager.dispatch_subprocess(['make'])
		self.process_manager.dispatch_subprocess(['make', 'install_sw', 'install_ssldirs'])


def main():
	parser = argparse.ArgumentParser(description='download and install catapult dependencies locally')
	parser.add_argument('--target', help='target dependencies directory', required=True)
	parser.add_argument('--versions', help='locked versions file', required=True)
	parser.add_argument('--download', help='download all dependencies', action='store_true')
	parser.add_argument('--build', help='build all dependencies', action='store_true')
	parser.add_argument('--use-clang', help='uses clang compiler instead of gcc', action='store_true')
	parser.add_argument('--dry-run', help='outputs desired commands without running them', action='store_true')
	parser.add_argument('--force', help='purges any existing files', action='store_true')
	args = parser.parse_args()

	versions = load_versions_map(args.versions)

	environment_manager = EnvironmentManager(args.dry_run)
	if args.force:
		environment_manager.rmtree(args.target)

	target_directory = Path(args.target).absolute()

	source_directory = Path(args.target) / SOURCE_DIR_NAME
	environment_manager.mkdirs(source_directory, exist_ok=True)
	environment_manager.chdir(source_directory)

	process_manager = ProcessManager(args.dry_run)

	dependency_repositories = [
		('google', 'googletest'),
		('google', 'benchmark'),
		('mongodb', 'mongo-c-driver'),
		('mongodb', 'mongo-cxx-driver'),
		('zeromq', 'libzmq'),
		('zeromq', 'cppzmq'),
		('facebook', 'rocksdb')
	]

	if args.download:
		print('[x] downloading all dependencies')
		downloader = Downloader(versions, process_manager)
		downloader.download_boost()
		downloader.download_git_dependency('openssl', 'openssl')

		for repository in dependency_repositories:
			downloader.download_git_dependency(repository[0], repository[1])

	if args.build:
		print('[x] building all dependencies')
		builder = Builder(target_directory, versions, process_manager, environment_manager)
		if args.use_clang:
			builder.use_clang()

		builder.build_boost()
		builder.build_openssl()

		for repository in dependency_repositories:
			builder.build_git_dependency(repository[0], repository[1])


if __name__ == '__main__':
	main()
