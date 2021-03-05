# python 3
# pylint: disable=too-few-public-methods

import argparse
from collections import defaultdict
from enum import Enum
import io
import os
import re
import shutil
import time

from xml.sax.saxutils import escape as xmlEscape

import colorama

import Parser
import validation
import HeaderParser
from DepsChecker import DepsChecker
from Rules import Rules, RULE_ID_TO_CLASS_MAP
from exclusions import SKIP_FILES, \
  NAMESPACES_FALSEPOSITIVES, \
  EMPTYLINES_FALSEPOSITIVES, \
  LONGLINES_FALSEPOSITIVES, \
  SPECIAL_INCLUDES, \
  CORE_FIRSTINCLUDES, \
  PLUGINS_FIRSTINCLUDES, \
  TOOLS_FIRSTINCLUDES, \
  EXTENSION_FIRSTINCLUDES, \
  FILTER_NAMESPACES

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
        self.textOutput = False
        self.fixIndents = False
        self.destDir = '.'
        self.verbose = False
        self.depsFilter = False
        self.depsSrcOnly = False


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


def isSpecialInclude(includePath):
    for filtered in SPECIAL_INCLUDES:
        if filtered.match(includePath):
            return True
    return False


class CheckResult(Enum):
    SUCCESS = 1
    MULTIPLE = 2
    INVALID = 3
    EMPTY = 4


def isExternalInclude(inc):
    return inc.startswith('<donna') or inc.startswith('<openssl')


def checkExternalInclude(incA, incB):
    extA = isExternalInclude(incA)
    extB = isExternalInclude(incB)

    if extA and not extB:
        return True

    if not extA and extB:
        return False

    return None


def isCppInclude(inc):
    cppIncludes = ['<boost', '<mongocxx', '<bsoncxx', '<rocksdb', '<benchmark']
    return any(map(inc.startswith, cppIncludes))


def checkCppInclude(incA, incB):
    boostA = isCppInclude(incA)
    boostB = isCppInclude(incB)

    if boostA and not boostB:
        return True

    if not boostA and boostB:
        return False

    return None


INCLUDE_PRIORITIES = {'"src': 100, '"mongo': 125, '"zeromq': 125, '"plugins': 150, '"catapult': 200, '"tests': 500}


def checkLocalInclude(pathA, pathB):
    partA = pathA[0]
    partB = pathB[0]
    valA = INCLUDE_PRIORITIES[partA] if partA in INCLUDE_PRIORITIES else 1
    valB = INCLUDE_PRIORITIES[partB] if partB in INCLUDE_PRIORITIES else 1

    if 'tests' in pathA:
        valA += 250
    if 'tests' in pathB:
        valB += 250

    if valA == valB:
        return None

    return valA < valB


def checkIncludeDepth(pathA, pathB):
    # single element goes to the top
    if len(pathA) == 1 and len(pathB) > 1:
        return True

    if len(pathA) > 1 and len(pathB) == 1:
        return False

    # two elements (i.e. "catapult/types.h") goes to bottom
    if len(pathA) == 2 and len(pathB) > 2:
        return False

    if len(pathA) > 2 and len(pathB) == 2:
        return True

    return None


class SortableInclude(HeaderParser.Include):
    def __init__(self, inc, ruleset):
        super().__init__(inc.line, inc.lineno, inc.include, inc.rest)
        self.ruleset = ruleset

    def comparePaths(self, other):
        incA = self.include
        incB = other.include

        # external first
        result = checkExternalInclude(incA, incB)
        if result is not None:
            return result

        # boost
        result = checkCppInclude(incA, incB)
        if result is not None:
            return result

        pathA = incA.split('/')
        pathB = incB.split('/')
        if incA[0] == '"':
            result = checkLocalInclude(pathA, pathB)
            if result is not None:
                return result

            result = checkIncludeDepth(pathA, pathB)
            if result is not None:
                return result
        else:
            pass

        return pathA < pathB

    def __eq__(self, other):
        return self.include == other.include

    def __lt__(self, other):
        if self.include[0] < other.include[0]:
            return True

        if self.include[0] > other.include[0]:
            return False

        if self.include[0] == '<':
            selfCHeader = self.include.endswith('.h>') and not isCppInclude(self.include)
            otherCHeader = other.include.endswith('.h>') and not isCppInclude(other.include)
            if selfCHeader and not otherCHeader:
                return False
            if (selfCHeader and otherCHeader) or not (selfCHeader or otherCHeader):
                return self.comparePaths(other)
            return True

        return self.comparePaths(other)


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
        self.templateErrors = None
        self.expectedNamespace = None
        splitted = re.split(r'[/\\]', self.fullPath())
        if splitted[0] == 'src':
            self.includeFixOwnPath = '/'.join(splitted[1:-1]) + '/'
        else:
            self.includeFixOwnPath = '/'.join(splitted[:-1]) + '/'

    def fullPath(self):
        return os.path.join(self.path, self.filename)

    def setIncludes(self, includes):
        self.includes = includes

    def setNamespaces(self, namespaces):
        self.namespaces = namespaces

    def setTemplateErrors(self, templateErrors):
        self.templateErrors = templateErrors

    def check(self):
        self.expectedNamespace = ''
        for filtered in NAMESPACES_FALSEPOSITIVES:
            if filtered.match(self.fullPath()):
                return CheckResult.SUCCESS

        if not self.namespaces:
            return CheckResult.EMPTY

        if len(self.namespaces) != 1:
            return CheckResult.MULTIPLE

        namespace = self.namespaces[0].name.split('::')
        namespace.append('')
        nsUnified = ':'.join(namespace)
        result = self.ruleset.namespaceCheck(nsUnified, self.fullPath())
        if isinstance(result, tuple):
            self.expectedNamespace = result[1]
            result = result[0]

        if result:
            return CheckResult.SUCCESS
        return CheckResult.INVALID

    def fixRelative(self, elem):
        temp = re.sub(self.includeFixOwnPath, '', elem.include)
        if '/' in temp:
            return

        elem.include = temp
        temp = re.sub(self.includeFixOwnPath, '', elem.line)
        elem.line = temp

    def checkCrossIncludes(self, errorReporter, sortedIncludes, pathElements):
        if 'tests' in pathElements:
            crossIncludes = self.ruleset.validateCrossIncludes(sortedIncludes, pathElements)
            if crossIncludes:
                errorReporter('crossIncludes', IncludesError(self.fullPath(), crossIncludes))

    def checkIncludes(self, errorReporter, preprocessor):
        sortedIncludes = []
        originalIncludes = []
        for elem in preprocessor:
            if elem.type != HeaderParser.PpType.INCLUDE:
                continue

            if isSpecialInclude(elem.include):
                continue

            originalIncludes.append(SortableInclude(elem, self.ruleset))
            self.fixRelative(elem)
            sortedIncludes.append(SortableInclude(elem, self.ruleset))

        fullPath = self.fullPath()
        pathElements = re.split(r'[/\\]', fullPath)
        sortedIncludes.sort()

        # move "own" header to first position (i.e. RemoteChainApi.cpp including RemoteChainApi.h)
        if fullPath.endswith('.cpp'):
            ownHeader = '<unknown>'
            # In case of both src and tests, there might not be predefined rule for first include,
            # but we still want includes to be sorted.
            # Pass sorted list of includes to checker, so it'll be allowed to return first element as first include
            if 'tests' in pathElements:
                ownHeader = self.ruleset.firstTestIncludeCheck(sortedIncludes, pathElements)
            else:
                ownHeader = self.ruleset.firstIncludeCheck(sortedIncludes, pathElements)

            if ownHeader != sortedIncludes[0].include:
                for i, elem in enumerate(sortedIncludes):
                    if ownHeader == elem.include:
                        sortedIncludes.insert(0, sortedIncludes.pop(i))
                        break

            if ownHeader != originalIncludes[0].include:
                errorReporter('firstInclude', FirstIncludeError(fullPath, ownHeader, originalIncludes[0].include))

            self.checkCrossIncludes(errorReporter, sortedIncludes, pathElements)

        if originalIncludes != sortedIncludes:
            print('I got includes mismatch')
            print('original', list(map(str, originalIncludes)))
            print('     new', list(map(str, sortedIncludes)))
            errorReporter('includesOrder', IncludesError(fullPath, sortedIncludes))


def namespaceFilter(namespace):
    for filtered in FILTER_NAMESPACES:
        if re.match(filtered, namespace.name):
            return False

        if namespace.hadForward and not (namespace.hadClass or namespace.hadFuncOrVar or namespace.hadUsing or namespace.hadTest):
            return False

        if namespace.empty():
            return False

    return True


def filterNonProjectIncludes(includes):
    projectIncludes = filter(lambda include: include[0] == '"', includes)
    projectIncludes = list(map(lambda name: name.strip('"'), projectIncludes))
    return projectIncludes


class ErrorDataCollector:
    def __init__(self, description):
        self.description = description

    def __call__(self, groupName, value):
        self.description[groupName].append(value)


class FilteredReporter:
    def __init__(self, description):
        self.xml = ErrorDataCollector(description)

    def __call__(self, groupName, err):
        skip = False
        name = os.path.basename(err.path)
        if 'emptyNearEnd' == groupName:
            for filtered in EMPTYLINES_FALSEPOSITIVES:
                if filtered.match(err.path):
                    skip = True
                    break
        elif 'tooLongLines' == groupName:
            for filtered in LONGLINES_FALSEPOSITIVES:
                if filtered.match(name):
                    skip = True
                    break
        if not skip:
            self.xml(groupName, err)


class ConReporter:
    def __init__(self):
        self.totalFailures = 0

    @staticmethod
    def formatFailure(buff, kind, name, msg):
        del kind, name
        buff.write(msg)

    def header(self, testsCount, failuresCount):
        pass

    def suite(self, suiteName, overallCount, errors):  # pylint: disable=no-self-use
        print('===== {} ===== (tests: {}, failures: {})'.format(suiteName, overallCount, len(errors)))
        self.totalFailures += len(errors)
        for err in errors:
            print(err)
        print('')

    def footer(self):
        print('>>> SUMMARY ({}, {} violations)'.format('SUCCESS' if 0 == self.totalFailures else 'FAILURE', self.totalFailures))
        print('')


class XmlReporter:
    def __init__(self, f):
        self.fout = f

    @staticmethod
    def formatFailure(buff, kind, name, msg):
        buff.write('  <testcase status="run" time="0" classname="{}" name="{}">\n'.format(kind, name))
        fixedMsg = xmlEscape(msg).replace('"', '&quot;')
        buff.write('    <failure message="{}" type=""><![CDATA[{}]]></failure>\n'.format(fixedMsg, msg))
        buff.write('  </testcase>\n')

    def header(self, testsCount, failuresCount):
        self.fout.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        formatStr = '<testsuites tests="{}" failures="{}" disabled="0" errors="0" time="0" name="AllTests">\n'
        self.fout.write(formatStr.format(testsCount, failuresCount))

    def suite(self, suiteName, overallCount, errors):
        formatStr = '  <testsuite name="{}" tests="{}" failures="{}" disabled="0" errors="0" time="0">\n'
        self.fout.write(formatStr.format(suiteName, overallCount, len(errors)))
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
        self.dependencyViolations = []
        self.conReporter = ConReporter()
        self.presentExclusions = defaultdict(set)
        self.options = options
        self.sourceDirs = []
        self.simpleValidators = validation.createValidators()

    @staticmethod
    def getShortestNamespaceSet(cppHeader):
        namespaces = set(cppHeader.namespaces)
        namespaces = list(filter(namespaceFilter, namespaces))

        if len(namespaces) <= 1:
            return namespaces

        # return only shortest NS if it's prefix of all other namespaces
        shortestNsLen = 100
        shortestNs = None
        for namespace in namespaces:
            nsLen = len(namespace.name.split('::'))
            if nsLen < shortestNsLen:
                shortestNsLen = nsLen
                shortestNs = namespace

        for namespace in namespaces:
            if not namespace.name.startswith(shortestNs.name):
                return namespaces

        return [shortestNs]

    def validateSet(self, mapName, path):
        for name in EXCLUSIONS[mapName]:
            if re.match(name, path):
                self.presentExclusions[mapName].add(name)

    def validateSetSearch(self, mapName, path):
        for name in EXCLUSIONS[mapName]:
            if re.search(name, path):
                self.presentExclusions[mapName].add(name)

    def validateSetPathFix(self, mapName, path):
        for name in EXCLUSIONS[mapName]:
            fix = name.replace('/', '.')
            if re.search(fix, path):
                self.presentExclusions[mapName].add(name)

    def validateMaps(self, path):
        self.validateSet('SKIP_FILES', path)
        self.validateSet('NAMESPACES_FALSEPOSITIVES', path)
        self.validateSet('EMPTYLINES_FALSEPOSITIVES', path)
        self.validateSetSearch('LONGLINES_FALSEPOSITIVES', path)
        self.validateSetPathFix('CORE_FIRSTINCLUDES', path)
        self.validateSetPathFix('PLUGINS_FIRSTINCLUDES', path)
        self.validateSetPathFix('TOOLS_FIRSTINCLUDES', path)
        self.validateSetPathFix('EXTENSION_FIRSTINCLUDES', path)

    def add(self, entry):
        path = entry.fullPath()
        self.validateMaps(path)

        for skipFile in SKIP_FILES:
            if skipFile.match(path):
                return

        if self.options.depsFilter:
            splittedDir = re.split(r'[/\\]', entry.path)
            if self.options.depsSrcOnly and splittedDir[0] != 'src':
                return

            if 'tests' in splittedDir or 'test' in splittedDir:
                return

        if self.options.verbose:
            print('parsing', entry.path, entry.filename)

        self.includes[path] = entry
        errorReporter = FilteredReporter(self.context)
        headers = HeaderParser.HeaderParser(errorReporter, path, self.simpleValidators, fixIndentsInFiles=self.options.fixIndents)

        entry.setIncludes(filterNonProjectIncludes(headers.includes))
        entry.checkIncludes(errorReporter, headers.preprocessor)

        namespaces = Parser.NamespacesParser(errorReporter, path)
        namespaceCandidates = self.getShortestNamespaceSet(namespaces)

        entry.setNamespaces(namespaceCandidates)
        entry.setTemplateErrors(namespaces.templateErrors)

    def printFormatting(self):
        if self.options.textOutput:
            self.printFormattingOut(self.conReporter)
            return

        with open(self.options.destDir + '/tests.formatting2.xml', 'w') as outputFile:
            reporter = XmlReporter(outputFile)
            self.printFormattingOut(reporter)

    def shouldIgnoreMissingExclusion(self, missingExcl):
        for sourceDir in self.sourceDirs:
            # extract pattern from either regex or string
            pattern = missingExcl.pattern if hasattr(missingExcl, 'pattern') else missingExcl
            if pattern.startswith(sourceDir):
                return False

        return True

    def getExclusionErrors(self, reporter):
        exclusionErrors = []
        for exclusionName, exclusionMap in EXCLUSIONS.items():
            if len(self.presentExclusions[exclusionName]) == len(exclusionMap):
                continue

            for missingExcl in exclusionMap:
                if missingExcl in self.presentExclusions[exclusionName]:
                    continue

                if self.shouldIgnoreMissingExclusion(missingExcl):
                    continue

                with io.StringIO() as output:
                    name = 'scripts/lint/exclusions.py'
                    msg = '{}: Exclusion from set: {}: >>{}<< was never hit'.format(name, exclusionName, missingExcl)
                    reporter.formatFailure(output, 'exclusions', name, msg)
                    exclusionErrors.append(output.getvalue())
        return exclusionErrors

    def printFormattingOut(self, reporter):
        exclusionErrors = self.getExclusionErrors(reporter)

        formatted = AutoContainer()
        for validator in self.simpleValidators:
            for err in self.context[validator.NAME]:
                with io.StringIO() as output:
                    name = err.path
                    reporter.formatFailure(output, validator.NAME, name, validator.formatError(err))
                    formatted[validator.NAME].append(output.getvalue())

        formats = {
            'consecutiveEmpty': lambda err: '{}:{} Consecutive empty lines: {}'.format(err.path, err.lineno, err.line),
            'preprocessorOther': lambda err: '{}:{} Preprocessor error, {}: {}'.format(err.path, err.lineno, err.kind, err.line),
            'anonNamespace': lambda err: '{}:{} Anonymous namespace inside header: {}'.format(err.path, err.lineno, err.line),
            'indentedPreprocessor': lambda err: '{}:{} Invalid indent, {}: {}'.format(err.path, err.lineno, err.kind, err.line),
            'emptyNearEnd': lambda err: '{}:{} Empty line near end of file: >>{}<<'.format(err.path, err.lineno, err.line),
            'firstInclude': lambda err: '{} Expected first include to be: >>{}<<'.format(err.path, err.include),
            'crossIncludes': lambda err: '{} Cross component includes: >>\n{}\n<<'.format(err.path, '\n'.join(map(str, err.includes))),
            'includesOrder':
                lambda err: '{} Includes needs fixing, proper order: >>\n{}\n<<'.format(err.path, '\n'.join(map(str, err.includes)))
        }
        for errorCategory, errorFormatter in formats.items():
            for err in self.context[errorCategory]:
                with io.StringIO() as output:
                    reporter.formatFailure(output, errorCategory, err.path, errorFormatter(err))
                    formatted[errorCategory].append(output.getvalue())

        failuresCount = 0
        for validator in self.simpleValidators:
            failuresCount += len(formatted[validator.NAME])

        successCount = len(self.includes)
        reporter.header(successCount, failuresCount)
        reporter.suite('Exclusions', successCount, exclusionErrors)
        for validator in self.simpleValidators:
            reporter.suite(validator.SUITE_NAME, len(self.includes), formatted[validator.NAME])
        for errorCategory in formats:
            reporter.suite(errorCategory.title(), successCount, formatted[errorCategory])
        reporter.footer()

    def printDependenciesOut(self, reporter):
        successCount = len(self.includes)
        failuresCount = len(self.dependencyViolations)
        reporter.header(successCount, failuresCount)
        formatted = []
        for err in self.dependencyViolations:
            with io.StringIO() as output:
                reporter.formatFailure(output, 'dependency', err[1], err[2])
                formatted.append(output.getvalue())
        reporter.suite('Dependencies', successCount, formatted)
        reporter.footer()

    def printDependencies(self):
        if self.options.textOutput:
            self.printDependenciesOut(self.conReporter)
            return

        with open(self.options.destDir + '/tests.dependencies.xml', 'w') as outputFile:
            reporter = XmlReporter(outputFile)
            self.printDependenciesOut(reporter)

    def filter(self, expectedResult):
        return filter(lambda elem: elem[1].check() == expectedResult, sorted(self.includes.items()))

    def printNamespaceErrorsOut(self, reporter):
        inconsistent = []
        multiple = []
        other = []
        for _, namespace in self.filter(CheckResult.INVALID):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                expected = '' if not namespace.expectedNamespace else ' expected >>{}<<'.format(namespace.expectedNamespace)
                msg = '{} namespace is inconsistent with file location: >>{}<<{}'.format(name, namespace.namespaces[0].name, expected)
                reporter.formatFailure(output, 'inconsistent', name, msg)
                inconsistent.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.MULTIPLE):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} Multiple: >>{}<<'.format(name, namespace.namespaces)
                reporter.formatFailure(output, 'multiple', name, msg)
                multiple.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.EMPTY):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} Couldn\'t figure out namespace'.format(name)
                reporter.formatFailure(output, 'empty', name, msg)
                other.append(output.getvalue())

        failuresCount = len(inconsistent) + len(multiple) + len(other)
        reporter.header(len(self.includes), failuresCount)
        reporter.suite('Inconsistent', len(self.includes), inconsistent)
        reporter.suite('Multiple', len(self.includes), multiple)
        reporter.suite('Other', len(self.includes), other)
        reporter.footer()

    def printNamespaces(self):
        if self.options.textOutput:
            self.printNamespaceErrorsOut(self.conReporter)
        else:
            with open(self.options.destDir + '/tests.namespaces.xml', 'w') as outputFile:
                reporter = XmlReporter(outputFile)
                self.printNamespaceErrorsOut(reporter)

    def printTemplateErrorsOut(self, reporter):
        failures = []
        for _, entry in sorted(self.includes.items()):
            for err in entry.templateErrors:
                with io.StringIO() as output:
                    name = os.path.join(entry.path, entry.filename)
                    msg = '{} class instead of typename used:\n{}'.format(name, err.line)
                    reporter.formatFailure(output, 'class', name, msg)
                    failures.append(output.getvalue())

        failuresCount = len(failures)
        reporter.header(len(self.includes), failuresCount)
        reporter.suite('Templates', len(self.includes), failures)
        reporter.footer()

    def printTemplateErrors(self):
        if self.options.textOutput:
            self.printTemplateErrorsOut(self.conReporter)
        else:
            with open(self.options.destDir + '/tests.templates.xml', 'w') as outputFile:
                reporter = XmlReporter(outputFile)
                self.printTemplateErrorsOut(reporter)


class DepsConsole:
    def __init__(self, args):
        self.args = args

    @staticmethod
    def includeFilter(path):
        return 'catapult' in path or 'plugins' in path or 'extensions' in path

    def depsPrint(self, deps):
        for name, entries in sorted(deps.items()):
            for dependencyName, files in sorted(entries.items()):
                if self.args.deps_nonCatapult and dependencyName.startswith('catapult'):
                    continue
                if dependencyName != 'catapult':
                    print('"{}" -> "{}"'.format(name, dependencyName))
                    for filename in files:
                        print('    - {}'.format(filename))

    def check(self, includes):
        print('---- ---- ----')
        print('Dependency map')
        deps = defaultdict(lambda: defaultdict(set))
        for name, entry in sorted(includes.items()):
            sourcePath = os.path.dirname(name)
            splittedSourcePath = re.split(r'[/\\]', sourcePath)
            if 'src' == splittedSourcePath[0]:
                pathA = '::'.join(splittedSourcePath[1:])
            else:
                pathA = '::'.join(splittedSourcePath)

            included = list(map(os.path.dirname, filter(DepsConsole.includeFilter, entry.includes)))
            if not included:
                continue
            for include in included:
                pathB = '::'.join(re.split(r'[/\\]', include))
                deps[pathA][pathB].add(name)

        self.depsPrint(deps)


def depsCheckDir(args, path):
    if not args.depCheckDir:
        return False

    for subdir in args.depCheckDir:
        if re.match(subdir, path):
            return 'tests' not in path

    return False


def checkDependencies(includes, depsChecker, args):
    for name, entry in sorted(includes.items()):
        sourcePath = os.path.dirname(name)
        splittedSourcePath = re.split(r'[/\\]', sourcePath)
        if 'src' == splittedSourcePath[0]:
            pathA = '/'.join(splittedSourcePath[1:])
        else:
            pathA = '/'.join(splittedSourcePath)

        if not depsCheckDir(args, name):
            continue

        for include in entry.includes:
            includeDir = os.path.dirname(include)
            if not includeDir:
                continue

            if depsChecker.match(name, pathA, includeDir, include):
                continue


def parseArgs():
    parser = argparse.ArgumentParser(description='catapult lint checker')
    parser.add_argument('-t', '--text', help='output text output (instead of xml)', action='store_true')
    parser.add_argument('-f', '--fixIndents', help='try to fix preprocessor indents', action='store_true')
    parser.add_argument('-v', '--verbose', help='output verbose parse progress', action='store_true')
    parser.add_argument('-d', '--depCheckDir', help='run dependency check on files matching pattern', action='append')
    parser.add_argument('-s', '--sourceDir', help='source directories to scan', action='append')
    parser.add_argument('--destDir', help='destination directory', action='store')

    sub = parser.add_subparsers(help='subcommands', dest='sub')
    helpMsg = 'console only dependency printer, shows dependency and file(s) that caused it. ' + \
        'Using deps printer disables analysis of test files'
    deps = sub.add_parser('deps', help=helpMsg)
    deps.add_argument('-s', '--srcOnly', help='causes to analyze only files under src',
                      action='store_true', dest='deps_srconly')
    deps.add_argument('-n', '--nonCatapult', help='show only `.* -> non-catapult` dependencies',
                      action='store_true', dest='deps_nonCatapult')

    return parser.parse_args()


def setupOptions(analyzerOptions, args):
    analyzerOptions.verbose = args.verbose
    if args.text:
        Parser.TEXT_OUTPUT = True
        analyzerOptions.textOutput = True
    if args.fixIndents:
        analyzerOptions.fixIndents = True
    if args.destDir:
        Parser.DEST_DIR = args.destDir
        analyzerOptions.destDir = args.destDir

    if args.sub == 'deps':
        analyzerOptions.depsFilter = True
        if args.deps_srconly:
            analyzerOptions.depsSrcOnly = True


def findAccessibleSourceDirs():
    sourceDirs = []
    for sourceDir in SOURCE_DIRS:
        if not os.path.exists(sourceDir):
            print('x skipping directory', sourceDir)
        else:
            sourceDirs.append(sourceDir)

    return sourceDirs


def printSectionSeparator():
    size = shutil.get_terminal_size()
    print()
    print('*' * size.columns)
    print()


def processDirectory(sourceDir, ruleset, analyzer):
    startTime = time.perf_counter()
    numFiles = 0
    print('> parsing directory', sourceDir)
    for root, _, files in os.walk(sourceDir):
        for filename in files:
            if not filename.endswith('.h') and not filename.endswith('.cpp'):
                continue

            analyzer.add(Entry(root, filename, ruleset))
            numFiles = numFiles + 1

    endTime = time.perf_counter()
    elapsedSeconds = endTime - startTime
    print('< elapsed {0:.3f}s {1} files ({2:.3f}ms / file)'.format(elapsedSeconds, numFiles, (elapsedSeconds * 1000) / numFiles))


def main():
    colorama.init()
    startTime = time.perf_counter()

    args = parseArgs()
    analyzerOptions = AnalyzerOptions()
    setupOptions(analyzerOptions, args)

    analyzer = Analyzer(analyzerOptions)
    analyzer.sourceDirs = args.sourceDir if args.sourceDir else findAccessibleSourceDirs()

    depsChecker = None
    if args.depCheckDir:
        depsChecker = DepsChecker('deps.config', analyzer.dependencyViolations, args.verbose)

    for sourceDir, ruleset in SOURCE_DIRS.items():
        if sourceDir not in analyzer.sourceDirs:
            continue

        processDirectory(sourceDir, ruleset, analyzer)

    printSectionSeparator()

    analyzer.printFormatting()
    analyzer.printNamespaces()
    analyzer.printTemplateErrors()

    if args.depCheckDir:
        checkDependencies(analyzer.includes, depsChecker, args)
        analyzer.printDependencies()

    # print dependencies on console
    if args.sub == 'deps':
        deps = DepsConsole(args)
        deps.check(analyzer.includes)

    endTime = time.perf_counter()
    elapsedSeconds = endTime - startTime
    print('*** lint elapsed {0:.3f}s ***'.format(elapsedSeconds))
    print()


if __name__ == '__main__':
    main()
