import argparse
from pathlib import Path

from configuration import load_versions_map
from dependency_flags import DEPENDENCY_FLAGS
from environment import EnvironmentManager
from process import ProcessManager

SOURCE_DIR_NAME = 'source'
NUM_BUILD_CORES = 8


class Downloader:
    def __init__(self, versions, process_manager):
        self.versions = versions
        self.process_manager = process_manager

    def download_boost(self):
        version = self.versions['boost']
        tar_filename = 'boost_1_{}_0.tar.gz'.format(version)
        tar_source_path = 'https://boostorg.jfrog.io/artifactory/main/release/1.{}.0/source/{}'.format(version, tar_filename)

        self.process_manager.dispatch_subprocess(['curl', '-o', tar_filename, '-SL', tar_source_path])
        self.process_manager.dispatch_subprocess(['tar', '-xzf', tar_filename])
        self.process_manager.dispatch_subprocess(['mv', 'boost_1_{}_0'.format(version), 'boost'])

    def download_git_dependency(self, organization, project):
        version = self.versions['{}_{}'.format(organization, project)]
        repository = 'git://github.com/{}/{}.git'.format(organization, project)
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

        boost_prefix_option = '--prefix={}'.format(self.target_directory / 'boost')
        bootstrap_options = ['./bootstrap.sh']
        if self.is_clang:
            bootstrap_options += ['with-toolset=clang']

        self.process_manager.dispatch_subprocess(bootstrap_options)

        b2_options = [boost_prefix_option]
        if self.is_clang:
            b2_options += ['toolset=clang', 'linkflags=\'-stdlib=libc++\'']

        self.process_manager.dispatch_subprocess(['./b2'] + b2_options + ['-j', str(NUM_BUILD_CORES), 'stage', 'release'])
        self.process_manager.dispatch_subprocess(['./b2', 'install'] + b2_options)

    def build_git_dependency(self, organization, project):
        self.environment_manager.chdir(self.target_directory / SOURCE_DIR_NAME / project)
        self.environment_manager.mkdirs('_build', exist_ok=True)
        self.environment_manager.chdir('_build')

        cmake_options = [
            'cmake', '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DCMAKE_INSTALL_PREFIX={}'.format(self.target_directory / organization)
        ]

        if self.is_clang:
            cmake_options += ['-DCMAKE_CXX_COMPILER=\'clang++\'', '-DCMAKE_CXX_FLAGS=\'-std=c++1y -stdlib=libc++\'']

        additional_cmake_options = DEPENDENCY_FLAGS.get('{}_{}'.format(organization, project), None)
        if additional_cmake_options:
            cmake_options += additional_cmake_options

        self.process_manager.dispatch_subprocess(cmake_options + ['..'])
        self.process_manager.dispatch_subprocess(['make', '-j', str(NUM_BUILD_CORES)])
        self.process_manager.dispatch_subprocess(['make', 'install'])


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

        for repository in dependency_repositories:
            downloader.download_git_dependency(repository[0], repository[1])

    if args.build:
        print('[x] building all dependencies')
        builder = Builder(target_directory, versions, process_manager, environment_manager)
        if args.use_clang:
            builder.use_clang()

        builder.build_boost()

        for repository in dependency_repositories:
            builder.build_git_dependency(repository[0], repository[1])


if __name__ == '__main__':
    main()
