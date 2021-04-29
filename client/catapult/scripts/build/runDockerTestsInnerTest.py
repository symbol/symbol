import argparse
import sys
from pathlib import Path

from environment import EnvironmentManager
from process import ProcessManager


def prepare_tests(environment_manager):
    environment_manager.copy_tree_with_symlinks('/catapult-src/seed', '/catapult-data/seed')
    environment_manager.copy_tree_with_symlinks('/catapult-src/resources', '/catapult-data/resources')

    environment_manager.mkdirs('/catapult-data/tests/int/stress')
    environment_manager.copy_tree_with_symlinks('/catapult-src/tests/int/stress/resources', '/catapult-data/tests/int/stress/resources')


def main():
    parser = argparse.ArgumentParser(description='catapult test runner')
    parser.add_argument('--exe-path', help='path to executables', required=True)
    parser.add_argument('--out-dir', help='directory in which to store result files', required=True)
    parser.add_argument('--verbosity', help='output verbosity', choices=('suite', 'test', 'max'), default='max')
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    process_manager = ProcessManager(args.dry_run)
    environment_manager = EnvironmentManager(args.dry_run)

    prepare_tests(environment_manager)

    process_manager.dispatch_subprocess(['ls', '-laF', '.'])
    process_manager.dispatch_subprocess(['ls', '-laF', '/catapult-data'])
    process_manager.dispatch_subprocess(['ls', '-laF', '/catapult-src'])

    failed_test_suites = []
    for filepath in environment_manager.find_glob(args.exe_path, 'tests*'):
        output_filepath = Path(args.out_dir) / (filepath.name + '.xml')
        test_args = [
            filepath,
            '--gtest_output=xml:{}'.format(output_filepath),
            Path(args.exe_path) / '..' / 'lib'
        ]
        if process_manager.dispatch_test_subprocess(test_args, args.verbosity):
            failed_test_suites.append(filepath)

    if failed_test_suites:
        print('test failures detected')
        for test_suite in sorted(failed_test_suites):
            print('[*] {}'.format(test_suite))

        sys.exit(len(failed_test_suites))
    else:
        print('all tests succeeded')


if __name__ == '__main__':
    main()
