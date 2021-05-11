import argparse
import sys
from pathlib import Path

from configuration import load_compiler_configuration
from environment import EnvironmentManager
from process import ProcessManager

USER_HOME = Path('/usr/catapult')
TSAN_SUPPRESSIONS_PATH = '/tmp/tsan-suppressions.txt'


class SanitizerEnvironment:
    def __init__(self, environment_manager, sanitizers):
        self.environment_manager = environment_manager
        self.sanitizers = sanitizers

    def prepare(self):
        if not self.sanitizers:
            return

        if 'thread' in self.sanitizers:
            self.prepare_thread_sanitizer()

        if 'undefined' in self.sanitizers:
            self.prepare_undefined_sanitizer()

        if 'address' in self.sanitizers:
            self.prepare_address_sanitizer()

    def prepare_thread_sanitizer(self):
        with open(TSAN_SUPPRESSIONS_PATH, 'wt') as outfile:
            outfile.write('race:~weak_ptr\n')
            outfile.write('race:~executor\n')
            outfile.write('race:global_logger::get()')

        options = {
            'external_symbolizer_path': str(USER_HOME / 'deps' / 'llvm-symbolizer'),
            'suppressions': TSAN_SUPPRESSIONS_PATH,
            'log_path': 'tsanlog'
        }

        options_string = ':'.join(map('{0[0]}={0[1]}'.format, options.items()))
        self.environment_manager.set_env_var('TSAN_OPTIONS', options_string)
        print('tsan options: {}'.format(options_string))

    def prepare_undefined_sanitizer(self):
        options = {
            'external_symbolizer_path': str(USER_HOME / 'deps' / 'llvm-symbolizer'),
            'print_stacktrace': 1,
            'log_path': 'ubsanlog'
        }
        options_string = ':'.join(map('{0[0]}={0[1]}'.format, options.items()))
        self.environment_manager.set_env_var('UBSAN_OPTIONS', options_string)
        print('ubsan options: {}'.format(options_string))

    def prepare_address_sanitizer(self):
        options = {
            'external_symbolizer_path': str(USER_HOME / 'deps' / 'llvm-symbolizer'),
            'verbosity': 1,
            'check_initialization_order': 'true',
            'strict_init_order': 'true',
            'detect_stack_use_after_return': 'true',
            'detect_container_overflow': 'false',
            'strict_string_checks': 'true',
            'new_delete_type_mismatch': 'false',
            'detect_leaks': 'true',
            'log_path': 'asanlog'
        }

        options_string = ':'.join(map('{0[0]}={0[1]}'.format, options.items()))
        self.environment_manager.set_env_var('ASAN_OPTIONS', options_string)
        print('asan options: {}'.format(options_string))


def prepare_tests(environment_manager):
    environment_manager.copy_tree_with_symlinks('/catapult-src/seed', '/catapult-data/seed')
    environment_manager.copy_tree_with_symlinks('/catapult-src/resources', '/catapult-data/resources')

    environment_manager.mkdirs('/catapult-data/tests/int/stress')
    environment_manager.copy_tree_with_symlinks('/catapult-src/tests/int/stress/resources', '/catapult-data/tests/int/stress/resources')


def create_san_parser_args(input_filepath, output_filepath, mode):
    return [
        'python3', '/catapult-src/scripts/build/san/sanParser.py',
        '--input={}'.format(input_filepath),
        '--output={}'.format(str(output_filepath)),
        '--mode={}'.format(mode)
    ]


def handle_sanitizer_logs(process_manager, output_directory, test_name):
    print('test_name', test_name)

    counter = 1
    for logfile_name in Path('.').glob('tsanlog*'):
        log_name = output_directory / '{}.thread.{}.xml'.format(test_name, counter)
        #process_manager.dispatch_subprocess(['cat', str(logfile_name)])
        process_manager.dispatch_subprocess(create_san_parser_args(logfile_name, log_name, 'tsan'))
        #process_manager.dispatch_subprocess(['cat', str(log_name)])
        counter += 1

    for logfile_name in Path('.').glob('tsanlog*'):
        logfile_name.unlink()

    process_manager.dispatch_subprocess(['ls', '-laF', '.'])

    counter = 1
    for logfile_name in Path('.').glob('ubsanlog*'):
        log_name = output_directory / '{}.undefined.address.{}.xml'.format(test_name, counter)
        #process_manager.dispatch_subprocess(['cat', str(logfile_name)])
        process_manager.dispatch_subprocess(create_san_parser_args(logfile_name, log_name, 'asan'))
        #process_manager.dispatch_subprocess(['cat', str(log_name)])
        counter += 1

    for logfile_name in Path('.').glob('ubsanlog*'):
        logfile_name.unlink()

    process_manager.dispatch_subprocess(['ls', '-laF', '.'])


def main():
    parser = argparse.ArgumentParser(description='catapult test runner')
    parser.add_argument('--compiler-configuration', help='path to compiler configuration yaml', required=True)
    parser.add_argument('--exe-path', help='path to executables', required=True)
    parser.add_argument('--out-dir', help='directory in which to store test output files', required=True)
    parser.add_argument('--verbosity', help='output verbosity', choices=('suite', 'test', 'max'), default='max')
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    process_manager = ProcessManager(args.dry_run)
    environment_manager = EnvironmentManager(args.dry_run)

    compiler_configuration = load_compiler_configuration(args.compiler_configuration)
    sanitizer_environment = SanitizerEnvironment(environment_manager, compiler_configuration.sanitizers)
    sanitizer_environment.prepare()

    prepare_tests(environment_manager)

    process_manager.dispatch_subprocess(['ls', '-laF', '.'])
    process_manager.dispatch_subprocess(['ls', '-laF', '/catapult-data'])
    process_manager.dispatch_subprocess(['ls', '-laF', '/catapult-src'])

    failed_test_suites = []
    for filepath in environment_manager.find_glob(args.exe_path, 'tests.catapult.thread*'):
        if '.int.' in str(filepath):
            print('skipping int tests')
            continue

        output_filepath = Path(args.out_dir) / (filepath.name + '.xml')
        test_args = [
            filepath,
            '--gtest_output=xml:{}'.format(output_filepath),
            Path(args.exe_path) / '..' / 'lib'
        ]
        if process_manager.dispatch_test_subprocess(test_args, args.verbosity):
            failed_test_suites.append(filepath)

        handle_sanitizer_logs(process_manager, Path(args.out_dir), filepath.name)

    if failed_test_suites:
        print('test failures detected')
        for test_suite in sorted(failed_test_suites):
            print('[*] {}'.format(test_suite))

        sys.exit(len(failed_test_suites))
    else:
        print('all tests succeeded')


if __name__ == '__main__':
    main()
