import argparse
import os
import shutil
from pathlib import Path

from BasicBuildManager import BasicBuildManager
from process import ProcessManager

CONAN_NEMTECH_REMOTE = 'https://catapult.jfrog.io/artifactory/api/conan/ngl-conan'


class LinuxEnvironment:
    def __init__(self, use_conan, process_manager):
        self.use_conan = use_conan
        self.dispatch_subprocess = process_manager.dispatch_subprocess

    def prepare(self):
        self._prepare_directory()
        self._prepare_environment_variables()

    @staticmethod
    def _prepare_directory():
        os.makedirs('/tmp/_build')
        os.chdir('/tmp/_build')

    def _prepare_environment_variables(self):
        if self.use_conan:
            # conan cache directory
            os.environ['HOME'] = '/conan'
        else:
            os.environ['BOOST_ROOT'] = '/mybuild'
            os.environ['GTEST_ROOT'] = '/usr/local'

    def prepare_conan(self, settings):
        # create default profile if it does not exist
        if self.dispatch_subprocess(['conan', 'profile', 'get', 'settings.compiler', 'default'], False):
            self.dispatch_subprocess(['conan', 'profile', 'new', 'default', '--detect'])

        self.dispatch_subprocess(['conan', 'remote', 'add', '--force', 'nemtech', CONAN_NEMTECH_REMOTE])
        self.dispatch_subprocess(['conan', 'config', 'set', 'general.revisions_enabled=True'])

        for key, value in settings.items():
            self.dispatch_subprocess(['conan', 'profile', 'update', 'settings.compiler.{}={}'.format(key, value), 'default'])

    def run_conan_install(self):
        # assuming working directory == build directory
        self.dispatch_subprocess(['conan', 'install', '/catapult-src', '--build', 'missing'])


class BuildManager(BasicBuildManager):
    def __init__(self, args, process_manager):
        super().__init__(args.compiler_configuration, args.build_configuration)
        self.dispatch_subprocess = process_manager.dispatch_subprocess

    def cmake_settings(self):
        settings = [
            ('CMAKE_INSTALL_PREFIX', '/binaries'),
            ('CMAKE_BUILD_TYPE', self.build_configuration),
            ('CATAPULT_TEST_DB_URL', 'mongodb://db:27017'),
            ('CATAPULT_DOCKER_TESTS', 'ON'),
            ('ENABLE_CODE_COVERAGE', 'OFF'),
            ('ARCHITECTURE_NAME', self.architecture)
        ]

        if self.enable_diagnostics:
            settings.append(('ENABLE_CATAPULT_DIAGNOSTICS', 'ON'))

        if self.use_conan:
            settings.append(('USE_CONAN', 'ON'))

        if self.sanitizers:
            settings.extend([
                # TODO: check if this works as expected
                ('USE_SANITIZER', ','.join(self.sanitizers)),
                ('OPENSSL_ROOT_DIR', '/usr/local')
            ])

        if self.is_release:
            settings.append(('CATAPULT_BUILD_RELEASE', 'ON'))
            settings.append(('ENABLE_TESTS', 'OFF'))

        return ['-D{}={}'.format(key, value) for key, value in settings]

    def run_cmake(self):
        cmake_settings = self.cmake_settings()
        self.dispatch_subprocess(['cmake'] + cmake_settings + ['-G', 'Ninja', '/catapult-src'])

    def build(self):
        self.dispatch_subprocess(['ninja', 'publish'])
        self.dispatch_subprocess(['ninja'])
        self.dispatch_subprocess(['ninja', 'install'])

    @staticmethod
    def _copy_with_symlinks(directory_path, pattern, destination):
        for file in Path(directory_path).glob(pattern):
            shutil.copy(file, destination, follow_symlinks=False)

    def copy_dependencies(self, destination):
        if self.use_conan:
            shutil.copytree('./deps', destination, symlinks=True)
            return

        os.makedirs(destination)
        for name in ['atomic', 'chrono', 'date_time', 'filesystem', 'log', 'log_setup', 'program_options', 'regex', 'thread']:
            self._copy_with_symlinks('/mybuild/lib', 'libboost_{}.so*'.format(name), destination)

        for name in ['bson-1.0', 'mongoc-1.0', 'bsoncxx', 'mongocxx', 'zmq', 'rocks', 'snappy', 'gflags']:
            self._copy_with_symlinks('/usr/lib/x86_64-linux-gnu', 'lib{}.so*'.format(name), destination)

    def copy_compiler_deps(self, destination):
        for dependency_pattern in self.compiler.deps:
            self._copy_with_symlinks(os.path.dirname(dependency_pattern), os.path.basename(dependency_pattern), destination)

    def copy_sanitizer_deps(self, destination):
        for name in ['crypto', 'ssl']:
            self._copy_with_symlinks('/usr/local/lib/', 'lib{}*'.format(name), destination)

        shutil.copytree('/usr/local/lib/engines-1.1', Path(destination) / 'engines-1.1', symlinks=True)

    def copy_files(self):
        deps_output_path = '/binaries/deps'
        tests_output_path = '/binaries/tests'

        # copy deps
        self.copy_dependencies(deps_output_path)
        self.copy_compiler_deps(deps_output_path)

        if self.sanitizers:
            self.copy_sanitizer_deps(deps_output_path)

        # copy tests
        if not self.is_release:
            os.makedirs(tests_output_path)
            self._copy_with_symlinks('./bin', 'tests*', tests_output_path)

        # list directories
        self.dispatch_subprocess(['ls', '-alh', deps_output_path])
        self.dispatch_subprocess(['ls', '-alh', tests_output_path])


def main():
    parser = argparse.ArgumentParser(description='catapult project build generator')
    parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
    parser.add_argument('--build-configuration', help='path to build configuration yaml', required=True)
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    process_manager = ProcessManager(args.dry_run)
    builder = BuildManager(args, process_manager)
    env = LinuxEnvironment(builder.use_conan, process_manager)

    if not args.dry_run:
        env.prepare()

    if builder.use_conan:
        env.prepare_conan({'version': builder.compiler.version, 'libcxx': builder.stl.lib})
        env.run_conan_install()

    builder.run_cmake()
    builder.build()
    if not args.dry_run:
        builder.copy_files()


if __name__ == '__main__':
    main()
