# python 3
# pylint: disable=too-few-public-methods

import argparse
import io
import os
import re
import shutil
import time
from collections import defaultdict
from enum import Enum
from xml.sax.saxutils import escape as xmlEscape

import colorama
import HeaderParser
import Parser
import validation
from DepsChecker import DepsChecker
from exclusions import (CORE_FIRSTINCLUDES, EMPTYLINES_FALSEPOSITIVES, EXTENSION_FIRSTINCLUDES, FILTER_NAMESPACES, LONGLINES_FALSEPOSITIVES,
                        NAMESPACES_FALSEPOSITIVES, PLUGINS_FIRSTINCLUDES, SKIP_FILES, SPECIAL_INCLUDES, TOOLS_FIRSTINCLUDES)
from Rules import RULE_ID_TO_CLASS_MAP, Rules

EXCLUSIONS = {
    'SKIP_FILES': SKIP_FILES,
    'NAMESPACES_FALSEPOSITIVES': NAMESPACES_FALSEPOSITIVES,
    'EMPTYLINES_FALSEPOSITIVES': EMPTYLINES_FALSEPOSITIVES,
    'LONGLINES_FALSEPOSITIVES': LONGLINES_FALSEPOSITIVES,
    'CORE_FIRSTINCLUDES': CORE_FIRSTINCLUDES,
    'PLUGINS_FIRSTINCLUDES': PLUGINS_FIRSTINCLUDES,
    'TOOLS_FIRSTINCLUDES': TOOLS_FIRSTINCLUDES,
    'EXTENSION_FIRSTINCLUDES': EXTENSION_FIRSTINCLUDES
}


class AnalyzerOptions():
    def __init__(self):
        self.text_output = False
        self.fix_indents = False
        self.dest_dir = '.'
        self.verbose = False
        self.deps_filter = False
        self.deps_src_only = False


USER_SOURCE_DIRS = {
    'src': Rules.DEFAULT,
    'sdk': Rules.PLUGIN,
    'tests': Rules.DEFAULT,
    'plugins': Rules.PLUGIN,
    'extensions': Rules.EXTENSION,
    'tools': Rules.TOOLS,
    'internal/tools': Rules.TOOLS
}


SOURCE_DIRS = dict((k, RULE_ID_TO_CLASS_MAP[v]) for k, v in USER_SOURCE_DIRS.items())


def is_special_include(include_path):
    for filtered in SPECIAL_INCLUDES:
        if filtered.match(include_path):
            return True
    return False


class CheckResult(Enum):
    SUCCESS = 1
    MULTIPLE = 2
    INVALID = 3
    EMPTY = 4


def is_external_include(inc):
    return inc.startswith('<donna') or inc.startswith('<openssl')


def check_external_include(inc_a, inc_b):
    ext_a = is_external_include(inc_a)
    ext_b = is_external_include(inc_b)

    if ext_a and not ext_b:
        return True

    if not ext_a and ext_b:
        return False

    return None


def is_cpp_include(inc):
    cpp_includes = ['<boost', '<mongocxx', '<bsoncxx', '<rocksdb', '<benchmark']
    return any(map(inc.startswith, cpp_includes))


def check_cpp_include(inc_a, inc_b):
    boost_a = is_cpp_include(inc_a)
    boost_b = is_cpp_include(inc_b)

    if boost_a and not boost_b:
        return True

    if not boost_a and boost_b:
        return False

    return None


INCLUDE_PRIORITIES = {'"src': 100, '"mongo': 125, '"zeromq': 125, '"plugins': 150, '"catapult': 200, '"tests': 500}


def check_local_include(path_a, path_b):
    part_a = path_a[0]
    part_b = path_b[0]
    val_a = INCLUDE_PRIORITIES[part_a] if part_a in INCLUDE_PRIORITIES else 1
    val_b = INCLUDE_PRIORITIES[part_b] if part_b in INCLUDE_PRIORITIES else 1

    if 'tests' in path_a:
        val_a += 250
    if 'tests' in path_b:
        val_b += 250

    if val_a == val_b:
        return None

    return val_a < val_b


def check_include_depth(path_a, path_b):
    # single element goes to the top
    if len(path_a) == 1 and len(path_b) > 1:
        return True

    if len(path_a) > 1 and len(path_b) == 1:
        return False

    # two elements (i.e. "catapult/types.h") goes to bottom
    if len(path_a) == 2 and len(path_b) > 2:
        return False

    if len(path_a) > 2 and len(path_b) == 2:
        return True

    return None


class SortableInclude(HeaderParser.Include):
    def __init__(self, inc, ruleset):
        super().__init__(inc.line, inc.lineno, inc.include, inc.rest)
        self.ruleset = ruleset

    def compare_paths(self, other):
        inc_a = self.include
        inc_b = other.include

        # external first
        result = check_external_include(inc_a, inc_b)
        if result is not None:
            return result

        # boost
        result = check_cpp_include(inc_a, inc_b)
        if result is not None:
            return result

        path_a = inc_a.split('/')
        path_b = inc_b.split('/')
        if inc_a[0] == '"':
            result = check_local_include(path_a, path_b)
            if result is not None:
                return result

            result = check_include_depth(path_a, path_b)
            if result is not None:
                return result
        else:
            pass

        return path_a < path_b

    def __eq__(self, other):
        return self.include == other.include

    def __lt__(self, other):
        if self.include[0] < other.include[0]:
            return True

        if self.include[0] > other.include[0]:
            return False

        if self.include[0] == '<':
            self_c_header = self.include.endswith('.h>') and not is_cpp_include(self.include)
            other_c_header = other.include.endswith('.h>') and not is_cpp_include(other.include)
            if self_c_header and not other_c_header:
                return False
            if (self_c_header and other_c_header) or not (self_c_header or other_c_header):
                return self.compare_paths(other)
            return True

        return self.compare_paths(other)


class FirstIncludeError:
    def __init__(self, path, include, actual):
        self.path = path
        self.include = include
        self.actual = actual


class IncludesError:
    def __init__(self, path, includes):
        self.path = path
        self.includes = includes


class Entry:
    def __init__(self, path, filename, ruleset):
        self.path = path
        self.filename = filename
        self.ruleset = ruleset
        self.includes = None
        self.namespaces = None
        self.template_errors = None
        self.expected_namespace = None
        splitted = re.split(r'[/\\]', self.full_path())
        if splitted[0] == 'src':
            self.include_fix_own_path = '/'.join(splitted[1:-1]) + '/'
        else:
            self.include_fix_own_path = '/'.join(splitted[:-1]) + '/'

    def full_path(self):
        return os.path.join(self.path, self.filename)

    def set_includes(self, includes):
        self.includes = includes

    def set_namespaces(self, namespaces):
        self.namespaces = namespaces

    def set_template_errors(self, template_errors):
        self.template_errors = template_errors

    def check(self):
        self.expected_namespace = ''
        for filtered in NAMESPACES_FALSEPOSITIVES:
            if filtered.match(self.full_path()):
                return CheckResult.SUCCESS

        if not self.namespaces:
            return CheckResult.EMPTY

        if len(self.namespaces) != 1:
            return CheckResult.MULTIPLE

        namespace = self.namespaces[0].name.split('::')
        namespace.append('')
        ns_unified = ':'.join(namespace)
        result = self.ruleset.namespace_check(ns_unified, self.full_path())
        if isinstance(result, tuple):
            self.expected_namespace = result[1]
            result = result[0]

        if result:
            return CheckResult.SUCCESS
        return CheckResult.INVALID

    def fix_relative(self, elem):
        temp = re.sub(self.include_fix_own_path, '', elem.include)
        if '/' in temp:
            return

        elem.include = temp
        temp = re.sub(self.include_fix_own_path, '', elem.line)
        elem.line = temp

    def check_cross_includes(self, error_reporter, sorted_includes, path_elements):
        if 'tests' in path_elements:
            cross_includes = self.ruleset.validate_cross_includes(sorted_includes, path_elements)
            if cross_includes:
                error_reporter('cross_includes', IncludesError(self.full_path(), cross_includes))

    def check_includes(self, error_reporter, preprocessor):
        sorted_includes = []
        original_includes = []
        for elem in preprocessor:
            if elem.type != HeaderParser.PpType.INCLUDE:
                continue

            if is_special_include(elem.include):
                continue

            original_includes.append(SortableInclude(elem, self.ruleset))
            self.fix_relative(elem)
            sorted_includes.append(SortableInclude(elem, self.ruleset))

        full_path = self.full_path()
        path_elements = re.split(r'[/\\]', full_path)
        sorted_includes.sort()

        # move "own" header to first position (i.e. RemoteChainApi.cpp including RemoteChainApi.h)
        if full_path.endswith('.cpp'):
            own_header = '<unknown>'
            # In case of both src and tests, there might not be predefined rule for first include,
            # but we still want includes to be sorted.
            # Pass sorted list of includes to checker, so it'll be allowed to return first element as first include
            if 'tests' in path_elements:
                own_header = self.ruleset.first_test_include_check(sorted_includes, path_elements)
            else:
                own_header = self.ruleset.first_include_check(sorted_includes, path_elements)

            if own_header != sorted_includes[0].include:
                for i, elem in enumerate(sorted_includes):
                    if own_header == elem.include:
                        sorted_includes.insert(0, sorted_includes.pop(i))
                        break

            if own_header != original_includes[0].include:
                error_reporter('firstInclude', FirstIncludeError(full_path, own_header, original_includes[0].include))

            self.check_cross_includes(error_reporter, sorted_includes, path_elements)

        if original_includes != sorted_includes:
            print('I got includes mismatch')
            print('original', list(map(str, original_includes)))
            print('     new', list(map(str, sorted_includes)))
            error_reporter('includesOrder', IncludesError(full_path, sorted_includes))


def namespace_filter(namespace):
    for filtered in FILTER_NAMESPACES:
        if re.match(filtered, namespace.name):
            return False

        if namespace.had_forward and not (namespace.had_class or namespace.had_func_or_var or namespace.had_using or namespace.had_test):
            return False

        if namespace.empty():
            return False

    return True


def filter_non_project_includes(includes):
    project_includes = filter(lambda include: include[0] == '"', includes)
    project_includes = list(map(lambda name: name.strip('"'), project_includes))
    return project_includes


class ErrorDataCollector:
    def __init__(self, description):
        self.description = description

    def __call__(self, group_name, value):
        self.description[group_name].append(value)


class FilteredReporter:
    def __init__(self, description):
        self.xml = ErrorDataCollector(description)

    def __call__(self, group_name, err):
        skip = False
        name = os.path.basename(err.path)
        if 'emptyNearEnd' == group_name:
            for filtered in EMPTYLINES_FALSEPOSITIVES:
                if filtered.match(err.path):
                    skip = True
                    break
        elif 'tooLongLines' == group_name:
            for filtered in LONGLINES_FALSEPOSITIVES:
                if filtered.match(name):
                    skip = True
                    break
        if not skip:
            self.xml(group_name, err)


class ConReporter:
    def __init__(self):
        self.total_failures = 0

    @staticmethod
    def format_failure(buff, kind, name, msg):
        del kind, name
        buff.write(msg)

    def header(self, tests_count, failures_count):
        pass

    def suite(self, suite_name, overall_count, errors):  # pylint: disable=no-self-use
        print('===== {} ===== (tests: {}, failures: {})'.format(suite_name, overall_count, len(errors)))
        self.total_failures += len(errors)
        for err in errors:
            print(err)
        print('')

    def footer(self):
        print('>>> SUMMARY ({}, {} violations)'.format('SUCCESS' if 0 == self.total_failures else 'FAILURE', self.total_failures))
        print('')


class XmlReporter:
    def __init__(self, f):
        self.fout = f

    @staticmethod
    def format_failure(buff, kind, name, msg):
        buff.write('  <testcase status="run" time="0" classname="{}" name="{}">\n'.format(kind, name))
        fixed_msg = xmlEscape(msg).replace('"', '&quot;')
        buff.write('    <failure message="{}" type=""><![CDATA[{}]]></failure>\n'.format(fixed_msg, msg))
        buff.write('  </testcase>\n')

    def header(self, tests_count, failures_count):
        self.fout.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        format_str = '<testsuites tests="{}" failures="{}" disabled="0" errors="0" time="0" name="AllTests">\n'
        self.fout.write(format_str.format(tests_count, failures_count))

    def suite(self, suite_name, overall_count, errors):
        format_str = '  <testsuite name="{}" tests="{}" failures="{}" disabled="0" errors="0" time="0">\n'
        self.fout.write(format_str.format(suite_name, overall_count, len(errors)))
        for err in errors:
            self.fout.write(err)
        self.fout.write('  </testsuite>\n')

    def footer(self):
        self.fout.write('</testsuites>\n')


class AutoContainer:
    def __init__(self):
        self.data = {}

    def __getitem__(self, key):
        if key not in self.data:
            self.data[key] = []
        return self.data[key]

    def __len__(self):
        return len(self.data)

    def __iter__(self):
        return self.data.__iter__()

    def items(self):
        return self.data.items()


class Analyzer:
    def __init__(self, options):
        self.includes = {}
        self.context = AutoContainer()
        self.dependency_violations = []
        self.con_reporter = ConReporter()
        self.present_exclusions = defaultdict(set)
        self.options = options
        self.source_dirs = []
        self.simple_validators = validation.create_validators()

    @staticmethod
    def get_shortest_namespace_set(cpp_header):
        namespaces = set(cpp_header.namespaces)
        namespaces = list(filter(namespace_filter, namespaces))

        if len(namespaces) <= 1:
            return namespaces

        # return only shortest NS if it's prefix of all other namespaces
        shortest_ns_len = 100
        shortest_ns = None
        for namespace in namespaces:
            ns_len = len(namespace.name.split('::'))
            if ns_len < shortest_ns_len:
                shortest_ns_len = ns_len
                shortest_ns = namespace

        for namespace in namespaces:
            if not namespace.name.startswith(shortest_ns.name):
                return namespaces

        return [shortest_ns]

    def validate_set(self, map_name, path):
        for name in EXCLUSIONS[map_name]:
            if re.match(name, path):
                self.present_exclusions[map_name].add(name)

    def validate_set_search(self, map_name, path):
        for name in EXCLUSIONS[map_name]:
            if re.search(name, path):
                self.present_exclusions[map_name].add(name)

    def validate_set_path_fix(self, map_name, path):
        for name in EXCLUSIONS[map_name]:
            fix = name.replace('/', '.')
            if re.search(fix, path):
                self.present_exclusions[map_name].add(name)

    def validate_maps(self, path):
        self.validate_set('SKIP_FILES', path)
        self.validate_set('NAMESPACES_FALSEPOSITIVES', path)
        self.validate_set('EMPTYLINES_FALSEPOSITIVES', path)
        self.validate_set_search('LONGLINES_FALSEPOSITIVES', path)
        self.validate_set_path_fix('CORE_FIRSTINCLUDES', path)
        self.validate_set_path_fix('PLUGINS_FIRSTINCLUDES', path)
        self.validate_set_path_fix('TOOLS_FIRSTINCLUDES', path)
        self.validate_set_path_fix('EXTENSION_FIRSTINCLUDES', path)

    def add(self, entry):
        path = entry.full_path()
        self.validate_maps(path)

        for skip_file in SKIP_FILES:
            if skip_file.match(path):
                return

        if self.options.deps_filter:
            splitted_dir = re.split(r'[/\\]', entry.path)
            if self.options.deps_src_only and splitted_dir[0] != 'src':
                return

            if 'tests' in splitted_dir or 'test' in splitted_dir:
                return

        if self.options.verbose:
            print('parsing', entry.path, entry.filename)

        self.includes[path] = entry
        error_reporter = FilteredReporter(self.context)
        headers = HeaderParser.HeaderParser(error_reporter, path, self.simple_validators, fix_indentsInFiles=self.options.fix_indents)

        entry.set_includes(filter_non_project_includes(headers.includes))
        entry.check_includes(error_reporter, headers.preprocessor)

        namespaces = Parser.NamespacesParser(error_reporter, path)
        namespace_candidates = self.get_shortest_namespace_set(namespaces)

        entry.set_namespaces(namespace_candidates)
        entry.set_template_errors(namespaces.template_errors)

    def print_formatting(self):
        if self.options.text_output:
            self.print_formatting_out(self.con_reporter)
            return

        with open(self.options.dest_dir + '/tests.formatting2.xml', 'w') as output_file:
            reporter = XmlReporter(output_file)
            self.print_formatting_out(reporter)

    def should_ignore_missing_exclusion(self, missing_excl):
        for source_dir in self.source_dirs:
            # extract pattern from either regex or string
            pattern = missing_excl.pattern if hasattr(missing_excl, 'pattern') else missing_excl
            if pattern.startswith(source_dir):
                return False

        return True

    def get_exclusion_errors(self, reporter):
        exclusion_errors = []
        for exclusion_name, exclusion_map in EXCLUSIONS.items():
            if len(self.present_exclusions[exclusion_name]) == len(exclusion_map):
                continue

            for missing_excl in exclusion_map:
                if missing_excl in self.present_exclusions[exclusion_name]:
                    continue

                if self.should_ignore_missing_exclusion(missing_excl):
                    continue

                with io.StringIO() as output:
                    name = 'scripts/lint/exclusions.py'
                    msg = '{}: Exclusion from set: {}: >>{}<< was never hit'.format(name, exclusion_name, missing_excl)
                    reporter.format_failure(output, 'exclusions', name, msg)
                    exclusion_errors.append(output.getvalue())
        return exclusion_errors

    def print_formatting_out(self, reporter):
        exclusion_errors = self.get_exclusion_errors(reporter)

        formatted = AutoContainer()
        for validator in self.simple_validators:
            for err in self.context[validator.NAME]:
                with io.StringIO() as output:
                    name = err.path
                    reporter.format_failure(output, validator.NAME, name, validator.format_error(err))
                    formatted[validator.NAME].append(output.getvalue())

        formats = {
            'consecutiveEmpty': lambda err: '{}:{} Consecutive empty lines: {}'.format(err.path, err.lineno, err.line),
            'preprocessorOther': lambda err: '{}:{} Preprocessor error, {}: {}'.format(err.path, err.lineno, err.kind, err.line),
            'anonNamespace': lambda err: '{}:{} Anonymous namespace inside header: {}'.format(err.path, err.lineno, err.line),
            'indentedPreprocessor': lambda err: '{}:{} Invalid indent, {}: {}'.format(err.path, err.lineno, err.kind, err.line),
            'emptyNearEnd': lambda err: '{}:{} Empty line near end of file: >>{}<<'.format(err.path, err.lineno, err.line),
            'firstInclude': lambda err: '{} Expected first include to be: >>{}<<'.format(err.path, err.include),
            'cross_includes': lambda err: '{} Cross component includes: >>\n{}\n<<'.format(err.path, '\n'.join(map(str, err.includes))),
            'includesOrder':
                lambda err: '{} Includes needs fixing, proper order: >>\n{}\n<<'.format(err.path, '\n'.join(map(str, err.includes)))
        }
        for error_category, error_formatter in formats.items():
            for err in self.context[error_category]:
                with io.StringIO() as output:
                    reporter.format_failure(output, error_category, err.path, error_formatter(err))
                    formatted[error_category].append(output.getvalue())

        failures_count = 0
        for validator in self.simple_validators:
            failures_count += len(formatted[validator.NAME])

        success_count = len(self.includes)
        reporter.header(success_count, failures_count)
        reporter.suite('Exclusions', success_count, exclusion_errors)
        for validator in self.simple_validators:
            reporter.suite(validator.SUITE_NAME, len(self.includes), formatted[validator.NAME])
        for error_category in formats:
            reporter.suite(error_category.title(), success_count, formatted[error_category])
        reporter.footer()

    def print_dependencies_out(self, reporter):
        success_count = len(self.includes)
        failures_count = len(self.dependency_violations)
        reporter.header(success_count, failures_count)
        formatted = []
        for err in self.dependency_violations:
            with io.StringIO() as output:
                reporter.format_failure(output, 'dependency', err[1], err[2])
                formatted.append(output.getvalue())
        reporter.suite('Dependencies', success_count, formatted)
        reporter.footer()

    def print_dependencies(self):
        if self.options.text_output:
            self.print_dependencies_out(self.con_reporter)
            return

        with open(self.options.dest_dir + '/tests.dependencies.xml', 'w') as output_file:
            reporter = XmlReporter(output_file)
            self.print_dependencies_out(reporter)

    def filter(self, expected_result):
        return filter(lambda elem: elem[1].check() == expected_result, sorted(self.includes.items()))

    def print_namespace_errors_out(self, reporter):
        inconsistent = []
        multiple = []
        other = []
        for _, namespace in self.filter(CheckResult.INVALID):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                expected = '' if not namespace.expected_namespace else ' expected >>{}<<'.format(namespace.expected_namespace)
                msg = '{} namespace is inconsistent with file location: >>{}<<{}'.format(name, namespace.namespaces[0].name, expected)
                reporter.format_failure(output, 'inconsistent', name, msg)
                inconsistent.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.MULTIPLE):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} Multiple: >>{}<<'.format(name, namespace.namespaces)
                reporter.format_failure(output, 'multiple', name, msg)
                multiple.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.EMPTY):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} Couldn\'t figure out namespace'.format(name)
                reporter.format_failure(output, 'empty', name, msg)
                other.append(output.getvalue())

        failures_count = len(inconsistent) + len(multiple) + len(other)
        reporter.header(len(self.includes), failures_count)
        reporter.suite('Inconsistent', len(self.includes), inconsistent)
        reporter.suite('Multiple', len(self.includes), multiple)
        reporter.suite('Other', len(self.includes), other)
        reporter.footer()

    def print_namespaces(self):
        if self.options.text_output:
            self.print_namespace_errors_out(self.con_reporter)
        else:
            with open(self.options.dest_dir + '/tests.namespaces.xml', 'w') as output_file:
                reporter = XmlReporter(output_file)
                self.print_namespace_errors_out(reporter)

    def print_template_errors_out(self, reporter):
        failures = []
        for _, entry in sorted(self.includes.items()):
            for err in entry.template_errors:
                with io.StringIO() as output:
                    name = os.path.join(entry.path, entry.filename)
                    msg = '{} class instead of typename used:\n{}'.format(name, err.line)
                    reporter.format_failure(output, 'class', name, msg)
                    failures.append(output.getvalue())

        failures_count = len(failures)
        reporter.header(len(self.includes), failures_count)
        reporter.suite('Templates', len(self.includes), failures)
        reporter.footer()

    def print_template_errors(self):
        if self.options.text_output:
            self.print_template_errors_out(self.con_reporter)
        else:
            with open(self.options.dest_dir + '/tests.templates.xml', 'w') as output_file:
                reporter = XmlReporter(output_file)
                self.print_template_errors_out(reporter)


class DepsConsole:
    def __init__(self, args):
        self.args = args

    @staticmethod
    def include_filter(path):
        return 'catapult' in path or 'plugins' in path or 'extensions' in path

    def deps_print(self, deps):
        for name, entries in sorted(deps.items()):
            for dependency_name, files in sorted(entries.items()):
                if self.args.deps_non_catapult and dependency_name.startswith('catapult'):
                    continue
                if dependency_name != 'catapult':
                    print('"{}" -> "{}"'.format(name, dependency_name))
                    for filename in files:
                        print('    - {}'.format(filename))

    def check(self, includes):
        print('---- ---- ----')
        print('Dependency map')
        deps = defaultdict(lambda: defaultdict(set))
        for name, entry in sorted(includes.items()):
            source_path = os.path.dirname(name)
            splitted_source_path = re.split(r'[/\\]', source_path)
            if 'src' == splitted_source_path[0]:
                path_a = '::'.join(splitted_source_path[1:])
            else:
                path_a = '::'.join(splitted_source_path)

            included = list(map(os.path.dirname, filter(DepsConsole.include_filter, entry.includes)))
            if not included:
                continue
            for include in included:
                path_b = '::'.join(re.split(r'[/\\]', include))
                deps[path_a][path_b].add(name)

        self.deps_print(deps)


def deps_check_dir(args, path):
    if not args.dep_check_dir:
        return False

    for subdir in args.dep_check_dir:
        if re.match(subdir, path):
            return 'tests' not in path

    return False


def check_dependencies(includes, deps_checker, args):
    for name, entry in sorted(includes.items()):
        source_path = os.path.dirname(name)
        splitted_source_path = re.split(r'[/\\]', source_path)
        if 'src' == splitted_source_path[0]:
            path_a = '/'.join(splitted_source_path[1:])
        else:
            path_a = '/'.join(splitted_source_path)

        if not deps_check_dir(args, name):
            continue

        for include in entry.includes:
            include_dir = os.path.dirname(include)
            if not include_dir:
                continue

            if deps_checker.match(name, path_a, include_dir, include):
                continue


def parse_args():
    parser = argparse.ArgumentParser(description='catapult lint checker')
    parser.add_argument('-t', '--text', help='output text output (instead of xml)', action='store_true')
    parser.add_argument('-f', '--fix-indents', help='try to fix preprocessor indents', action='store_true')
    parser.add_argument('-v', '--verbose', help='output verbose parse progress', action='store_true')
    parser.add_argument('-d', '--dep-check-dir', help='run dependency check on files matching pattern', action='append')
    parser.add_argument('-s', '--source-dir', help='source directories to scan', action='append')
    parser.add_argument('--dest-dir', help='destination directory', action='store')

    sub = parser.add_subparsers(help='subcommands', dest='sub')
    help_msg = 'console only dependency printer, shows dependency and file(s) that caused it. ' + \
        'Using deps printer disables analysis of test files'
    deps = sub.add_parser('deps', help=help_msg)
    deps.add_argument('-s', '--src-only', help='causes to analyze only files under src',
                      action='store_true', dest='deps_src_only')
    deps.add_argument('-n', '--non-catapult', help='show only `.* -> non-catapult` dependencies',
                      action='store_true', dest='deps_non_catapult')

    return parser.parse_args()


def setup_options(analyzer_options, args):
    analyzer_options.verbose = args.verbose
    if args.text:
        Parser.TEXT_OUTPUT = True
        analyzer_options.text_output = True
    if args.fix_indents:
        analyzer_options.fix_indents = True
    if args.dest_dir:
        Parser.DEST_DIR = args.dest_dir
        analyzer_options.dest_dir = args.dest_dir

    if args.sub == 'deps':
        analyzer_options.deps_filter = True
        if args.deps_src_only:
            analyzer_options.deps_src_only = True


def find_accessible_source_dirs():
    source_dirs = []
    for source_dir in SOURCE_DIRS:
        if not os.path.exists(source_dir):
            print('x skipping directory', source_dir)
        else:
            source_dirs.append(source_dir)

    return source_dirs


def print_section_separator():
    size = shutil.get_terminal_size()
    print()
    print('*' * size.columns)
    print()


def process_directory(source_dir, ruleset, analyzer):
    start_time = time.perf_counter()
    num_files = 0
    print('> parsing directory', source_dir)
    for root, _, files in os.walk(source_dir):
        for filename in files:
            if not filename.endswith('.h') and not filename.endswith('.cpp'):
                continue

            analyzer.add(Entry(root, filename, ruleset))
            num_files = num_files + 1

    end_time = time.perf_counter()
    elapsed_seconds = end_time - start_time
    print('< elapsed {0:.3f}s {1} files ({2:.3f}ms / file)'.format(elapsed_seconds, num_files, (elapsed_seconds * 1000) / num_files))


def main():
    colorama.init()
    start_time = time.perf_counter()

    args = parse_args()
    analyzer_options = AnalyzerOptions()
    setup_options(analyzer_options, args)

    analyzer = Analyzer(analyzer_options)
    analyzer.source_dirs = args.source_dir if args.source_dir else find_accessible_source_dirs()

    deps_checker = None
    if args.dep_check_dir:
        deps_checker = DepsChecker('deps.config', analyzer.dependency_violations, args.verbose)

    for source_dir, ruleset in SOURCE_DIRS.items():
        if source_dir not in analyzer.source_dirs:
            continue

        process_directory(source_dir, ruleset, analyzer)

    print_section_separator()

    analyzer.print_formatting()
    analyzer.print_namespaces()
    analyzer.print_template_errors()

    if args.dep_check_dir:
        check_dependencies(analyzer.includes, deps_checker, args)
        analyzer.print_dependencies()

    # print dependencies on console
    if args.sub == 'deps':
        deps = DepsConsole(args)
        deps.check(analyzer.includes)

    end_time = time.perf_counter()
    elapsed_seconds = end_time - start_time
    print('*** lint elapsed {0:.3f}s ***'.format(elapsed_seconds))
    print()


if __name__ == '__main__':
    main()
