import argparse
import sys
from pathlib import Path

from configuration import load_compiler_configuration, load_versions_map
from dependency_flags import get_dependency_flags

SINGLE_COMMAND_SEPARATOR = ' \\\n    && '
LAYER_TO_IMAGE_TAG_MAP = {'os': 'preimage1', 'boost': 'preimage2', 'deps': 'preimage3', 'test': '', 'conan': 'conan'}
VS_DEV_CMD = r'C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
BOOST_DISABLED_LIBS = map(lambda library_name: f'--without-{library_name}', [
	'context',
	'contract',
	'coroutine',
	'fiber',
	'graph',
	'graph_parallel',
	'headers',
	'iostreams',
	'json',
	'mpi',
	'nowide',
	'python',
	'serialization',
	'stacktrace',
	'test',
	'timer',
	'type_erasure',
	'wave'
])


def print_line(lines, **kwargs):
	print(SINGLE_COMMAND_SEPARATOR.join(lines).format(**kwargs))


def print_line_with_continuation(lines, **kwargs):
	print(SINGLE_COMMAND_SEPARATOR.join(lines).format(**kwargs), end='')
	print(SINGLE_COMMAND_SEPARATOR, end='')


def print_lines(lines, **kwargs):
	print('\n'.join(lines).format(**kwargs))


def format_multivalue_options(key, values):
	return f'{key}=\'{" ".join(values)}\''


def print_powershell_lines(lines, separator='; `\n', **kwargs):
	run_line = 'RUN pwsh -Command $ErrorActionPreference = \'Stop\''
	print(separator.join([run_line] + lines).format(**kwargs))


def print_msvc_line(lines, separator=' `\n    && ', **kwargs):
	run_line = f'RUN {VS_DEV_CMD}'
	print(separator.join([run_line] + lines).format(**kwargs))


def install_pip_package(user, package_name):
	print_lines([
		f'USER {user}',
		f'RUN python3 -m pip install -U {package_name}',
		'USER root'
	])


# region OptionsManager

class OptionsManager:
	class OptionsDescriptor:
		def __init__(self):
			self.options = []
			self.cxxflags = []
			self.linkflags = []
			self.sanitizer = None

	def __init__(self, compiler_configuration, operating_system, versions_filepath, ignore_architecture):
		self.compiler = compiler_configuration.compiler
		self.operating_system = operating_system
		self.sanitizers = compiler_configuration.sanitizers
		self.architecture = compiler_configuration.architecture
		self.stl = compiler_configuration.stl
		self.ignore_architecture = ignore_architecture

		self.versions = load_versions_map(versions_filepath)

	@property
	def is_clang(self):
		return self.compiler.c.startswith('clang')

	@property
	def is_msvc(self):
		return self.compiler.c.startswith('msvc')

	@property
	def base_image_name(self):
		name_parts = [self.operating_system, self.compiler.c, str(self.compiler.version)]
		if not self.ignore_architecture:
			name_parts.append(self.architecture)

		return f'symbolplatform/symbol-server-compiler:{"-".join(name_parts)}'

	def layer_image_name(self, layer):
		name_parts = [self.operating_system, self.compiler.c, str(self.compiler.version)]
		if 'conan' != layer:
			name_parts.extend(self.sanitizers)

		tag = LAYER_TO_IMAGE_TAG_MAP[layer]
		if tag:
			name_parts.append(tag)

		if not self.ignore_architecture:
			name_parts.append(self.architecture)

		return f'symbolplatform/symbol-server-build-base:{"-".join(name_parts)}'

	def bootstrap(self):
		options = []
		if self.is_clang:
			options += ['--with-toolset=clang']

		return options

	def b2(self):  # pylint: disable=invalid-name
		options = []
		cxxflags = [self._arch_flag] if not self.is_msvc else []
		if self.is_clang:
			options += ['toolset=clang']
			options += [format_multivalue_options('linkflags', [f'-stdlib={self.stl.lib}'])]
			cxxflags += self._stl_flags

		options += get_dependency_flags('boost')
		options += [format_multivalue_options('cxxflags', cxxflags)]
		return options

	def openssl(self):
		if self.is_msvc:
			return []

		return [format_multivalue_options('CFLAGS', [self._arch_flag])]

	def openssl_configure(self):
		if 'address' in self.sanitizers:
			return ['enable-asan']

		if 'undefined' in self.sanitizers:
			return ['enable-ubsan']

		return []

	def mongo_c(self):
		descriptor = self._enable_thread_san_descriptor()
		descriptor.options += ['-DOPENSSL_ROOT_DIR=/usr/catapult/deps']
		descriptor.options += get_dependency_flags('mongodb_mongo-c-driver')

		return self._cmake(descriptor)

	def mongo_cxx(self):
		descriptor = self._enable_thread_san_descriptor()
		descriptor.options += ['-DOPENSSL_ROOT_DIR=/usr/catapult/deps']
		descriptor.options += get_dependency_flags('mongodb_mongo-cxx-driver')
		descriptor.options += [f'-DBUILD_VERSION={self.versions["mongodb_mongo-cxx-driver"][1:]}']

		if self.is_msvc:
			# For build without a C++17 polyfill
			# https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
			descriptor.cxxflags += ['/Zc:__cplusplus']

		return self._cmake(descriptor)

	def libzmq(self):
		descriptor = self._enable_thread_san_descriptor()
		descriptor.options += get_dependency_flags('zeromq_libzmq')

		if self.is_clang:
			# Xeon-based build machine, even with -mskylake seems to do miscompilation in libzmq,
			# try to pass additional flags to disable faulty optimizations
			descriptor.cxxflags += ['-mno-avx', '-mno-avx2']

		return self._cmake(descriptor)

	def cppzmq(self):
		descriptor = self._enable_thread_san_descriptor()
		descriptor.options += get_dependency_flags('zeromq_cppzmq')
		return self._cmake(descriptor)

	def _enable_thread_san_descriptor(self):
		descriptor = self.OptionsDescriptor()
		if 'thread' in self.sanitizers:
			descriptor.sanitizer = 'thread'

		return descriptor

	def rocks(self):
		descriptor = self.OptionsDescriptor()
		descriptor.options += get_dependency_flags('facebook_rocksdb')
		descriptor.options += ['-DUSE_RTTI=1']

		# Disable warning as error due to a bug in gcc which should be fix in 12.2
		# https://github.com/facebook/rocksdb/issues/9925
		if self.compiler.c.startswith('gcc') and 12 == self.compiler.version:
			descriptor.cxxflags += ['-Wno-error=maybe-uninitialized']

		if self.compiler.c.startswith('clang') and 15 == self.compiler.version:
			descriptor.cxxflags += ['-Wno-error=unused-but-set-variable']

		return self._cmake(descriptor)

	def googletest(self):
		descriptor = self.OptionsDescriptor()
		descriptor.options += get_dependency_flags('google_googletest')
		descriptor.sanitizer = ','.join(self.sanitizers)
		return self._cmake(descriptor)

	def googlebench(self):
		descriptor = self.OptionsDescriptor()
		descriptor.options += get_dependency_flags('google_benchmark')
		if self.compiler.c.startswith('clang') and 15 == self.compiler.version:
			descriptor.cxxflags += ['-Wno-error=unused-but-set-variable']

		return self._cmake(descriptor)

	@property
	def _arch_flag(self):
		if 'arm64' == self.architecture:
			return ''

		return f'-march={self.architecture}'

	@property
	def _stl_flags(self):
		return None if not self.stl else [f'-std={self.stl.version}', f'-stdlib={self.stl.lib}']

	def _cmake(self, descriptor):
		descriptor.options += ['-DCMAKE_BUILD_TYPE=RelWithDebInfo']
		if not self.is_msvc:
			descriptor.cxxflags += [self._arch_flag]
			descriptor.options += ['-DCMAKE_INSTALL_PREFIX=/usr']

		if self.is_clang:
			descriptor.options += [format_multivalue_options('-DCMAKE_CXX_COMPILER', [self.compiler.cpp])]
			descriptor.cxxflags += self._stl_flags

		if descriptor.sanitizer:
			sanitize_flag = f'-fsanitize={descriptor.sanitizer}'
			descriptor.options += [format_multivalue_options('-DCMAKE_C_FLAGS', [sanitize_flag])]
			descriptor.cxxflags += [sanitize_flag]
			descriptor.linkflags += [sanitize_flag]

		if descriptor.cxxflags:
			descriptor.options += [format_multivalue_options('-DCMAKE_CXX_FLAGS', descriptor.cxxflags)]

		if descriptor.linkflags:
			descriptor.options += [
				format_multivalue_options('-DCMAKE_SHARED_LINKER_FLAGS', descriptor.linkflags),
				format_multivalue_options('-DCMAKE_EXE_LINKER_FLAGS', descriptor.linkflags)
			]

		return descriptor.options


# endregion

# region SYSTEMS

class UbuntuSystem:
	@staticmethod
	def user():
		return 'ubuntu'

	@staticmethod
	def add_base_os_packages():
		apt_packages = [
			'autoconf',
			'ca-certificates',
			'ccache',
			'curl',
			'gdb',
			'git',
			'libatomic-ops-dev',
			'libgflags-dev',
			'libsnappy-dev',
			'libtool',
			'make',
			'ninja-build',
			'pkg-config',
			'python3',
			'python3-pip',
			'python3-venv',
			'python3-ply',
			'xz-utils'
		]
		print_line_with_continuation([
			'RUN apt-get -y update',
			'apt-get install -y {APT_PACKAGES}',
			'rm -rf /var/lib/apt/lists/*'
		], APT_PACKAGES=' '.join(apt_packages))

	@staticmethod
	def add_test_packages(user, install_openssl):
		apt_packages = ['python3-pip', 'lcov']
		if install_openssl:
			apt_packages += ['libssl-dev']

		print_line([
			'RUN apt-get -y update',
			'apt-get remove -y --purge pylint',
			'apt-get install -y {APT_PACKAGES}'
		], APT_PACKAGES=' '.join(apt_packages))
		install_pip_package(user, 'pycodestyle pylint pyyaml')


class FedoraSystem:
	@staticmethod
	def user():
		return 'fedora'

	@staticmethod
	def add_base_os_packages():
		rpm_packages = [
			'ccache',
			'curl',
			'gdb',
			'gflags-devel',
			'git',
			'libunwind-devel',
			'make',
			'ninja-build',
			'perl-core',
			'python3',
			'xz'
		]
		print_line_with_continuation([
			'RUN dnf update --assumeyes',
			'dnf install --assumeyes {RPM_PACKAGES}',
			'dnf clean all',
			'rm -rf /var/cache/yum'
		], RPM_PACKAGES=' '.join(rpm_packages))

	@staticmethod
	def add_test_packages(user, install_openssl):
		rpm_packages = ['python3-pip']
		if install_openssl:
			rpm_packages += ['openssl-devel']

		print_line([
			'RUN dnf update --assumeyes',
			'dnf remove --assumeyes pylint',
			'dnf install --assumeyes {RPM_PACKAGES}',
			'dnf clean all',
			'rm -rf /var/cache/yum'
		], RPM_PACKAGES=' '.join(rpm_packages))
		install_pip_package(user, 'pycodestyle pylint pyyaml')


class WindowsSystem:
	@staticmethod
	def add_base_os_packages():
		scoop_packages = [
			'ccache',
			'git',
			'python',
			'cmake',
			'7zip'
		]
		print_powershell_lines([
			'scoop update',
			'scoop install {SCOOP_PACKAGES}'
		], SCOOP_PACKAGES=' '.join(scoop_packages))

	@staticmethod
	def add_test_packages():
		print_powershell_lines([
			'scoop update',
			'python3 -m pip install -U pycodestyle pylint pyyaml'
		])


SYSTEMS = {'ubuntu': UbuntuSystem, 'debian': UbuntuSystem, 'fedora': FedoraSystem, 'windows': WindowsSystem}


class LinuxSystemGenerator:
	def __init__(self, system, options):
		self.system = system
		self.options = options

	def generate_phase_os(self):
		# for compiler ignore architecture since we dont have a westmere compiler
		self.options.ignore_architecture = True
		print_lines([
			'FROM {BASE_IMAGE_NAME}',
			'ARG DEBIAN_FRONTEND=noninteractive',
			'LABEL maintainer="Catapult Development Team"'
		], BASE_IMAGE_NAME=self.options.base_image_name)

		self.system.add_base_os_packages()

		cmake_version = self.options.versions['cmake']
		cmake_platform = 'aarch64' if 'arm64' == self.options.architecture else 'x86_64'

		cmake_script = f'cmake-{cmake_version}-Linux-{cmake_platform}.sh'
		cmake_uri = f'https://github.com/Kitware/CMake/releases/download/v{cmake_version}'
		print_line([
			'curl -o {CMAKE_SCRIPT} -SL "{CMAKE_URI}/{CMAKE_SCRIPT}"',
			'chmod +x {CMAKE_SCRIPT}',
			'./{CMAKE_SCRIPT} --skip-license --prefix=/usr',
			'rm -rf {CMAKE_SCRIPT}'
		], CMAKE_SCRIPT=cmake_script, CMAKE_URI=cmake_uri)

		# create a virtual python environment
		print_lines([
			f'# add user {self.system.user()} (used by jenkins) if it does not exist',
			f'RUN id -u "{self.system.user()}" || useradd --uid 1000 -ms /bin/bash {self.system.user()}',
			f'USER {self.system.user()}',
			f'WORKDIR /home/{self.system.user()}',
			f'ENV VIRTUAL_ENV=/home/{self.system.user()}/venv',
			'RUN python3 -m venv $VIRTUAL_ENV',
			'ENV PATH="$VIRTUAL_ENV/bin:$PATH"',
			'USER root'
		])

	def generate_phase_boost(self):
		print(f'FROM {self.options.layer_image_name("os")}')
		gosu_version = self.options.versions['gosu']
		gosu_target = '/usr/local/bin/gosu'
		gosu_uri = f'https://github.com/tianon/gosu/releases/download/{gosu_version}'
		print_line([
			'RUN curl -o {GOSU_TARGET} -SL "{GOSU_URI}/gosu-$(dpkg --print-architecture)"',
			'chmod +x {GOSU_TARGET}'
		], GOSU_TARGET=gosu_target, GOSU_URI=gosu_uri)

		boost_version = self.options.versions['boost']

		print_args = {
			'BOOST_ARCHIVE': f'boost_{boost_version.replace(".", "_")}',
			'BOOST_URI': f'https://boostorg.jfrog.io/artifactory/main/release/{boost_version}/source',
			'BOOTSTRAP_OPTIONS': ' '.join(self.options.bootstrap()),
			'B2_OPTIONS': ' '.join(self.options.b2()),
			'BOOST_DISABLED_LIBS': ' '.join(BOOST_DISABLED_LIBS)
		}
		print_line([
			'RUN curl -o {BOOST_ARCHIVE}.tar.gz -SL {BOOST_URI}/{BOOST_ARCHIVE}.tar.gz',
			'tar -xzf {BOOST_ARCHIVE}.tar.gz',
			'mkdir /mybuild',
			'cd {BOOST_ARCHIVE}',
			'./bootstrap.sh {BOOTSTRAP_OPTIONS} --prefix=/mybuild',
			'./b2 {B2_OPTIONS} --prefix=/mybuild {BOOST_DISABLED_LIBS} -j 8 stage release',
			'./b2 {B2_OPTIONS} {BOOST_DISABLED_LIBS} install'
		], **print_args)

	def add_git_dependency(self, organization, project, options, revision=1):
		version = self.options.versions[f'{organization}_{project}']
		print_line([
			'RUN git clone https://github.com/{ORGANIZATION}/{PROJECT}.git -b {VERSION}',
			'cd {PROJECT}',
			'mkdir _build',
			'cd _build',
			'cmake {OPTIONS} ..',
			'make -j 8',
			'make install',
			'cd ..',
			'rm -rf {PROJECT}',
			'echo \"force rebuild revision {REVISION}\"'
		], ORGANIZATION=organization, PROJECT=project, VERSION=version, OPTIONS=' '.join(options), REVISION=revision)

	@staticmethod
	def add_openssl(options, configure):
		version = options.versions['openssl_openssl']
		compiler = 'linux-aarch64' if 'arm64' == options.architecture else 'linux-x86_64-clang' if options.is_clang else 'linux-x86_64'
		openssl_destinations = [f'--{key}=/usr/catapult/deps' for key in ('prefix', 'openssldir', 'libdir')]
		print_line([
			'RUN git clone https://github.com/openssl/openssl.git -b {VERSION}',
			'cd openssl',
			'{OPENSSL_OPTIONS} perl ./Configure {COMPILER} {OPENSSL_CONFIGURE} {OPENSSL_DESTINATIONS}',
			'make -j 8',
			'make install_sw install_ssldirs',
			'cd ..',
			'rm -rf openssl'
		],
			OPENSSL_OPTIONS=' '.join(options.openssl()),
			OPENSSL_CONFIGURE=' '.join(configure),
			OPENSSL_DESTINATIONS=' '.join(openssl_destinations),
			VERSION=version,
			COMPILER=compiler)

	def generate_phase_deps(self):
		print(f'FROM {self.options.layer_image_name("boost")}')

		self.add_openssl(self.options, [])

		self.add_git_dependency('mongodb', 'mongo-c-driver', self.options.mongo_c())
		self.add_git_dependency('mongodb', 'mongo-cxx-driver', self.options.mongo_cxx())

		self.add_git_dependency('zeromq', 'libzmq', self.options.libzmq())
		self.add_git_dependency('zeromq', 'cppzmq', self.options.cppzmq())

		self.add_git_dependency('facebook', 'rocksdb', self.options.rocks())

	def generate_phase_test(self):
		print(f'FROM {self.options.layer_image_name("deps")}')
		self.add_git_dependency('google', 'googletest', self.options.googletest())
		self.add_git_dependency('google', 'benchmark', self.options.googlebench())

		self.system.add_test_packages(self.system.user(), not self.options.sanitizers)

		self.add_openssl(self.options, self.options.openssl_configure() if self.options.sanitizers else [])

		print_lines([
			'RUN echo "docker image build $BUILD_NUMBER"',
			'CMD ["/bin/bash"]'
		])

	def generate_phase_conan(self):
		print(f'FROM {self.options.layer_image_name("os")}')

		apt_packages = ['python3-pip']

		print_line([
			'RUN apt-get -y update',
			'apt-get install -y {APT_PACKAGES}'
		], APT_PACKAGES=' '.join(apt_packages))
		install_pip_package(self.system.user(), 'conan')


class WindowsSystemGenerator:
	def __init__(self, system, options):
		self.system = system
		self.options = options
		self.deps_path = Path('c:/usr/catapult/deps')

	def generate_phase_os(self):
		print_lines([
			'# escape=`',
			'FROM {BASE_IMAGE_NAME}',
			'LABEL maintainer="Catapult Development Team"'
		], BASE_IMAGE_NAME=self.options.base_image_name)

		self.system.add_base_os_packages()

	def generate_phase_boost(self):
		print('# escape=`')
		print(f'FROM {self.options.layer_image_name("os")}')

		boost_version = self.options.versions['boost']
		print_args = {
			'BOOST_ARCHIVE': f'boost_{boost_version.replace(".", "_")}',
			'BOOST_URI': f'https://boostorg.jfrog.io/artifactory/main/release/{boost_version}/source',
			'BOOTSTRAP_OPTIONS': ' '.join(self.options.bootstrap()),
			'B2_OPTIONS': ' '.join(self.options.b2()),
			'BOOST_DISABLED_LIBS': ' '.join(BOOST_DISABLED_LIBS),
			'PREFIX_PATH': self.deps_path / 'boost'
		}

		print_powershell_lines([
			'Invoke-WebRequest {BOOST_URI}/{BOOST_ARCHIVE}.7z -outfile {BOOST_ARCHIVE}.7z',
			r'7z x .\{BOOST_ARCHIVE}.7z',
			r'cd {BOOST_ARCHIVE}',
			r'.\bootstrap.bat {BOOTSTRAP_OPTIONS} --prefix={PREFIX_PATH}',
			r'.\b2 {B2_OPTIONS} --prefix={PREFIX_PATH} {BOOST_DISABLED_LIBS} -j 8 stage release',
			r'.\b2 {B2_OPTIONS} --prefix={PREFIX_PATH} {BOOST_DISABLED_LIBS} install',
			'cd ..',
			'del {BOOST_ARCHIVE}.7z',
			'Remove-Item {BOOST_ARCHIVE} -Recurse -Force',
		], **print_args)

	def add_git_dependency(self, organization, project, package_options, revision=1):
		version = self.options.versions[f'{organization}_{project}']
		generator = 'Visual Studio 16 2019' if 16 == self.options.compiler.version else 'Visual Studio 17 2022'
		prefix_path = self.deps_path / organization
		print_msvc_line([
			'git clone https://github.com/{ORGANIZATION}/{PROJECT}.git -b {VERSION}',
			'cd {PROJECT}',
			'mkdir _build',
			'cd _build',
			'cmake {OPTIONS} -S .. -G "{GENERATOR}" -A x64 -DCMAKE_INSTALL_PREFIX={PREFIX_PATH} -DCMAKE_PREFIX_PATH={PREFIX_PATH}',
			'cmake --build . -j 8 --config RelWithDebInfo --target install',
			'cd ../..',
			'rmdir /q /s {PROJECT}',
			'echo \"force rebuild revision {REVISION}\"'
		],
			ORGANIZATION=organization,
			PROJECT=project,
			VERSION=version,
			OPTIONS=' '.join(package_options),
			REVISION=revision,
			PREFIX_PATH=prefix_path,
			GENERATOR=generator)

	def add_openssl(self, package_options, configure):
		version = self.options.versions['openssl_openssl']
		openssl_destinations = [f'--{key}={self.deps_path / "openssl"}' for key in ('prefix', 'openssldir')]
		print_msvc_line([
			'git clone https://github.com/openssl/openssl.git -b {VERSION}',
			'cd openssl',
			'{OPENSSL_OPTIONS} perl ./Configure VC-WIN64A {OPENSSL_CONFIGURE} {OPENSSL_DESTINATIONS}',
			'nmake',
			'nmake install_sw install_ssldirs',
			'cd ..',
			'rmdir /q /s openssl'
		],
			OPENSSL_OPTIONS=' '.join(package_options),
			OPENSSL_CONFIGURE=' '.join(configure),
			OPENSSL_DESTINATIONS=' '.join(openssl_destinations),
			VERSION=version)

	def generate_phase_deps(self):
		print('# escape=`')
		print(f'FROM {self.options.layer_image_name("boost")}')

		print_powershell_lines([
			'scoop install nasm perl'
		])

		self.add_openssl(self.options.openssl(), self.options.openssl_configure())

		self.add_git_dependency('mongodb', 'mongo-c-driver', self.options.mongo_c())
		self.add_git_dependency('mongodb', 'mongo-cxx-driver', self.options.mongo_cxx())

		self.add_git_dependency('zeromq', 'libzmq', self.options.libzmq())
		self.add_git_dependency('zeromq', 'cppzmq', self.options.cppzmq())

		self.add_git_dependency('facebook', 'rocksdb', self.options.rocks())

	def generate_phase_test(self):
		print('# escape=`')
		print(f'FROM {self.options.layer_image_name("deps")}')
		self.add_git_dependency('google', 'googletest', self.options.googletest())
		self.add_git_dependency('google', 'benchmark', self.options.googlebench())

		self.system.add_test_packages()

		print_lines([
			'RUN echo "docker image build $BUILD_NUMBER"'
		])

	def generate_phase_conan(self):
		print('# escape=`')
		print(f'FROM {self.options.layer_image_name("os")}')

		print_powershell_lines([
			'scoop update',
			'python3 -m pip install -U conan',
			'echo "docker image build $BUILD_NUMBER"'
		])


def main():
	parser = argparse.ArgumentParser(description='catapult base image dockerfile generator')
	parser.add_argument('--layer', help='name of docker layer to generate', choices=LAYER_TO_IMAGE_TAG_MAP.keys(), required=True)
	parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
	parser.add_argument('--operating-system', help='operating system', required=True)
	parser.add_argument('--versions', help='locked versions file', required=True)
	parser.add_argument('--name-only', help='true to output layer name', action='store_true')
	parser.add_argument('--base-name-only', help='true to output base name', action='store_true')
	parser.add_argument('--ignore-architecture', help='ignore architecture for image name', action='store_true')
	args = parser.parse_args()

	compiler_configuration = load_compiler_configuration(args.compiler_configuration)
	options_manager = OptionsManager(compiler_configuration, args.operating_system, args.versions, args.ignore_architecture)

	if args.base_name_only:
		print(options_manager.base_image_name)
		return

	if args.name_only:
		print(options_manager.layer_image_name(args.layer))
		return

	if args.ignore_architecture:
		print('error: ignore architecture can only be used with name-only or base-name-only')
		sys.exit(1)

	system_generator_type = WindowsSystemGenerator if 'windows' == args.operating_system else LinuxSystemGenerator
	dockerfile_generator = system_generator_type(SYSTEMS[args.operating_system], options_manager)
	{
		'os': dockerfile_generator.generate_phase_os,
		'boost': dockerfile_generator.generate_phase_boost,
		'deps': dockerfile_generator.generate_phase_deps,
		'test': dockerfile_generator.generate_phase_test,

		'conan': dockerfile_generator.generate_phase_conan
	}[args.layer]()


if __name__ == '__main__':
	main()
