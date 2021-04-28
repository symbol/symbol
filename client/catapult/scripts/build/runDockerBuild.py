import argparse
import os
import shutil
import sys
from pathlib import Path

from BasicBuildManager import BasicBuildManager
from process import ProcessManager

CCACHE_ROOT = '/home/ubuntu/jenkins/ccache'
CONAN_HOST = '/home/ubuntu/jenkins/conan-'

OUTPUT_DIR = Path('..') / 'output'
BINARIES_DIR = OUTPUT_DIR / 'binaries'


class OptionsManager(BasicBuildManager):
    def __init__(self, args):
        super().__init__(args.compiler_configuration, args.build_configuration)

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
    def base_image_name(self):
        if self.use_conan:
            return 'symbolplatform/symbol-server-build-base:{}-{}-{}'.format(self.compiler.c, self.compiler.version, 'conan')

        return 'symbolplatform/symbol-server-build-base:{}'.format(self.compilation_friendly_name)

    @property
    def ccache_path(self):
        return Path(CCACHE_ROOT) / ('release' if self.is_release else 'all')

    @property
    def conan_path(self):
        return Path(CONAN_HOST + ('clang' if self.is_clang else 'gcc'))

    def docker_run_settings(self):
        settings = [
            ('IMAGE_TYPE', self.image_type),

            ('CC', self.compiler.c),
            ('CXX', self.compiler.cpp),

            ('BUILD_VISIBILITY', 'none'),

            ('CCACHE_DIR', '/ccache')
        ]

        if self.enable_diagnostics:
            settings.append(('ENABLE_CATAPULT_DIAGNOSTICS', 'ON'))

        if self.sanitizers:
            settings.append(('USE_SANITIZER', ''.join(self.sanitizers)))

        return ['--env={}={}'.format(key, value) for key, value in sorted(settings)]


def get_volume_mappings(ccache_path, conan_path):
    mappings = [
        (Path('').resolve(), '/catapult-src'),
        (BINARIES_DIR.resolve(), '/binaries'),
        (conan_path, '/conan'),
        (ccache_path, '/ccache')
    ]

    return ['--volume={}:{}'.format(str(key), value) for key, value in sorted(mappings)]


def create_docker_run_command(options, compiler_configuration_filepath, build_configuration_filepath, user):
    docker_run_settings = options.docker_run_settings()
    volume_mappings = get_volume_mappings(options.ccache_path, options.conan_path)

    docker_args = [
        'docker', 'run',
        '--rm',
        '--user={}'.format(user),
    ] + docker_run_settings + volume_mappings + [
        options.base_image_name,
        'python3',
        '/catapult-src/scripts/build/buildCatapultProject.py',
        # assume paths are relative...
        '--compiler-configuration=/catapult-src/{}'.format(compiler_configuration_filepath),
        '--build-configuration=/catapult-src/{}'.format(build_configuration_filepath)
    ]

    return docker_args


def rm_failure_handler(func, path, excinfo):
    del func
    del path
    if excinfo[0] != FileNotFoundError:
        raise excinfo[1]


def cleanup_directories(ccache_host_directory, conan_host_directory):
    shutil.rmtree(OUTPUT_DIR, onerror=rm_failure_handler)
    os.makedirs(BINARIES_DIR)

    os.makedirs(ccache_host_directory, exist_ok=True)
    os.makedirs(conan_host_directory, exist_ok=True)


def main():
    parser = argparse.ArgumentParser(description='catapult project build generator')
    parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
    parser.add_argument('--build-configuration', help='path to build configuration yaml', required=True)
    parser.add_argument('--user', help='docker user', required=True)
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    options = OptionsManager(args)
    docker_run = create_docker_run_command(options, args.compiler_configuration, args.build_configuration, args.user)

    if not args.dry_run:
        cleanup_directories(options.ccache_path / 'tmp', options.conan_path)

    # run build

    process_manager = ProcessManager(args.dry_run)

    process_manager.dispatch_subprocess(['docker', 'pull', options.base_image_name])
    return_code = process_manager.dispatch_subprocess(docker_run)
    if return_code:
        sys.exit(return_code)

    # copy files

    # if not args.dry_run:
    #     print('copying files')
    #     catapult_src_path = Path('').resolve()
    #     os.chdir(OUTPUT_DIR)

    #     for folder_name in ['scripts', 'seed', 'resources']:
    #         os.mkdir(folder_name)
    #         shutil.copytree(catapult_src_path / folder_name, folder_name)

    #     if (catapult_src_path / 'internal').is_dir():
    #         pass

    # prepare image

    print('prepared image')
    sys.exit(0)


if __name__ == '__main__':
    main()
