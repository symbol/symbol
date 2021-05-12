import argparse
import sys
from pathlib import Path

from configuration import load_compiler_configuration
from environment import EnvironmentManager
from process import ProcessManager
from sanParser import parse_san_log

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

    def _set_san_options(self, name, custom_options):
        options = {
            'external_symbolizer_path': str(USER_HOME / 'deps' / 'llvm-symbolizer'),
            'print_stacktrace': 1,
            'verbosity': 1,
            'log_path': '{}log'.format(name)
        }
        options.update(custom_options)

        options_string = ':'.join(map('{0[0]}={0[1]}'.format, options.items()))
        self.environment_manager.set_env_var('{}_OPTIONS'.format(name.upper()), options_string)
        print('{} options: {}'.format(name, options_string))

    def prepare_thread_sanitizer(self):
        with open(TSAN_SUPPRESSIONS_PATH, 'wt') as outfile:
            outfile.write('race:~weak_ptr\n')
            outfile.write('race:~executor\n')
            outfile.write('race:global_logger::get()')

        self._set_san_options('tsan', {
            'suppressions': TSAN_SUPPRESSIONS_PATH,
        })

    def prepare_undefined_sanitizer(self):
        self._set_san_options('ubsan', {})

    def prepare_address_sanitizer(self):
        self._set_san_options('asan', {
            'check_initialization_order': 'true',
            'strict_init_order': 'true',
            'detect_stack_use_after_return': 'true',
            'detect_container_overflow': 'false',
            'strict_string_checks': 'true',
            'new_delete_type_mismatch': 'false',
            'detect_leaks': 'true',
        })


def prepare_tests(environment_manager):
    environment_manager.copy_tree_with_symlinks('/catapult-src/seed', '/catapult-data/seed')
    environment_manager.copy_tree_with_symlinks('/catapult-src/resources', '/catapult-data/resources')

    environment_manager.mkdirs('/catapult-data/tests/int/stress')
    environment_manager.copy_tree_with_symlinks('/catapult-src/tests/int/stress/resources', '/catapult-data/tests/int/stress/resources')


def process_sanitizer_logs(environment_manager, output_directory, san_descriptor):
    counter = 1
    for logfile_name in environment_manager.find_glob('.', san_descriptor['input_pattern']):
        log_name = output_directory / '{}.{}.xml'.format(san_descriptor['output_filename_prefix'], counter)
        parse_san_log(logfile_name, log_name, san_descriptor['parser_name'])

        logfile_name.unlink()
        counter += 1


def process_sanitizer_logs_all(environment_manager, output_directory, test_name):
    print('test_name', test_name)

    process_sanitizer_logs(environment_manager, output_directory, {
        'input_pattern': 'tsanlog*',
        'output_filename_prefix': '{}.thread'.format(test_name),
        'parser_name': 'tsan'
    })
    process_sanitizer_logs(environment_manager, output_directory, {
        'input_pattern': 'ubsanlog*',
        'output_filename_prefix': '{}.undefined.address'.format(test_name),
        'parser_name': 'asan'
    })


SEGV_RESULT_TEMPLATE = '''<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="1" failures="1" disabled="0" errors="0" timestamp="0" time="0" name="AllTests">
    <testsuite name="Segmentation" tests="1" failures="1" disabled="0" errors="0" time="0">
        <testcase name="SegmentationFault" status="run" time="0.002" classname="unknown">
            <failure message="Segmentation fault happened" type=""><![CDATA[{}]]></failure>
        </testcase>
    </testsuite>
</testsuites>
'''


def handle_core_file(process_manager, core_path, test_exe_filepath, base_output_filepath):
    print('core file detected {}'.format(core_path))

    gdb_output_filepath = '{}.core.txt'.format(base_output_filepath)
    process_manager.dispatch_subprocess([
        'gdb', '--batch', '--quiet',
        '-ex', 'thread apply all bt full',
        '-ex', 'quit',
        test_exe_filepath,
        core_path
    ], redirect_filename=gdb_output_filepath)

    with open(gdb_output_filepath, 'rt') as infile:
        contents = infile.read()

    with open('{}.core.xml'.format(base_output_filepath), 'wt') as outfile:
        outfile.write(SEGV_RESULT_TEMPLATE.format(contents))


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
    for test_exe_filepath in environment_manager.find_glob(args.exe_path, 'tests*'):
        base_output_filepath = Path(args.out_dir) / test_exe_filepath.name

        output_filepath = '{}.xml'.format(base_output_filepath)
        test_args = [
            test_exe_filepath,
            '--gtest_output=xml:{}'.format(output_filepath),
            Path(args.exe_path) / '..' / 'lib'
        ]

        if process_manager.dispatch_test_subprocess(test_args, args.verbosity):
            for core_path in Path('.').glob('core*'):
                handle_core_file(process_manager, core_path, test_exe_filepath, base_output_filepath)

            failed_test_suites.append(test_exe_filepath)

        process_sanitizer_logs_all(environment_manager, Path(args.out_dir), test_exe_filepath.name)

    if failed_test_suites:
        print('test failures detected')
        for test_suite in sorted(failed_test_suites):
            print('[*] {}'.format(test_suite))

        sys.exit(len(failed_test_suites))
    else:
        print('all tests succeeded')


if __name__ == '__main__':
    main()
