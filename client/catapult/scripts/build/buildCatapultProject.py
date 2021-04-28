import argparse
import os

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

        # if self.exclude_tests:
        #    settings.append(('ENABLE_TESTS', 'OFF'))

        if self.is_release:
            settings.append(('CATAPULT_BUILD_RELEASE', 'ON'))

        return ['-D{}={}'.format(key, value) for key, value in settings]

    def run_cmake(self):
        cmake_settings = self.cmake_settings()
        self.dispatch_subprocess(['cmake'] + cmake_settings + ['-G', 'Ninja', '/catapult-src'])

    def build(self):
        self.dispatch_subprocess(['ninja', 'publish'])
        self.dispatch_subprocess(['ninja'])


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


if __name__ == '__main__':
    main()
