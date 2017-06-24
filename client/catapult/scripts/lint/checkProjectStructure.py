# python 3
# pylint: disable=too-few-public-methods

from collections import defaultdict
from enum import Enum
import io
import os
import re
import sys

from xml.sax.saxutils import escape as xmlEscape

import colorama

import Parser
import validation
import HeaderParser
from exclusions import SKIP_FILES, NAMESPACES_FALSEPOSITIVES, EMPTYLINES_FALSEPOSITIVES, LONGLINES_FALSEPOSITIVES, SPECIAL_INCLUDES, CORE_FIRSTINCLUDES, PLUGINS_FIRSTINCLUDES, TOOLS_FIRSTINCLUDES # pylint: disable=line-too-long

EXCLUSIONS = {
    'SKIP_FILES': SKIP_FILES,
    'NAMESPACES_FALSEPOSITIVES': NAMESPACES_FALSEPOSITIVES,
    'EMPTYLINES_FALSEPOSITIVES': EMPTYLINES_FALSEPOSITIVES,
    'LONGLINES_FALSEPOSITIVES': LONGLINES_FALSEPOSITIVES,
    'CORE_FIRSTINCLUDES': CORE_FIRSTINCLUDES,
    'PLUGINS_FIRSTINCLUDES': PLUGINS_FIRSTINCLUDES,
    'TOOLS_FIRSTINCLUDES': TOOLS_FIRSTINCLUDES
}

TEXT_OUTPUT = False
FIX_INDENTS = False
if len(sys.argv) > 1:
    if '--text' in sys.argv:
        Parser.TEXT_OUTPUT = True
        TEXT_OUTPUT = True
    if '--fixIndents' in sys.argv:
        FIX_INDENTS = True

class Rules(Enum):
    Default = 1
    Plugin = 2
    Tools = 3

USER_SOURCE_DIRS = {
    'src' : Rules.Default,
    'sdk' : Rules.Plugin,
    'tests' : Rules.Default,
    'plugins' : Rules.Plugin,
    'tools' : Rules.Tools
}

class DefaultRules:
    @staticmethod
    def namespaceCheck(nsUnified, fullPath):
        splittedPath = re.split(r'[/\\]', fullPath)
        pathUnified = ':'.join(splittedPath)
        if nsUnified == 'catapult:test:':
            if 'tests:test:' in pathUnified:
                return True
            if 'tests' in splittedPath and 'utils' in splittedPath:
                return True
            # hack for TestHarness.h, as it's the only file that's directly inside tests/ directory,
            # but it has namespace catapult::test
            if 'tests:TestHarness.h' == pathUnified:
                return True
            return False
        elif nsUnified == 'catapult:mocks:':
            nsUnified = ':mocks:'
            if re.search(r':Mock', pathUnified):
                return True

        return nsUnified in pathUnified

    @staticmethod
    def firstIncludeCheck(pathElements):
        fullPath = '/'.join(pathElements)
        if fullPath in CORE_FIRSTINCLUDES:
            return '"{}"'.format(CORE_FIRSTINCLUDES[fullPath])
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(originalIncludes, pathElements):
        fullPath = '/'.join(pathElements)
        if 'int' in pathElements:
            return originalIncludes[0].include

        if pathElements[0] != 'tests':
            return '<firstTestIncludeCheck called on non test path>' + fullPath

        if fullPath in CORE_FIRSTINCLUDES:
            includePath = CORE_FIRSTINCLUDES[fullPath]
            if includePath[0] == '<':
                return includePath
            return '"{}"'.format(includePath)

        if pathElements[-1].endswith('Tests.cpp'):
            includeDir = '/'.join(pathElements[1:-1])
            return '"{}/{}.h"'.format(includeDir, pathElements[-1][:-9])
        return '"{}.h"'.format(pathElements[-1][:-4])

class PluginRules:
    @staticmethod
    def namespaceCheck(nsUnified, fullPath):
        pathElements = re.split(r'[/\\]', fullPath)
        if 'src' in pathElements:
            elementId = pathElements.index('src')
        elif 'int' in pathElements:
            elementId = pathElements.index('int')
        elif 'tests' in pathElements:
            elementId = pathElements.index('tests')
        else:
            return False

        expectedUnified = 'catapult:plugins:'
        if 'mongo' in pathElements:
            expectedUnified = 'catapult:mongo:plugins:'

        nextPart = pathElements[elementId + 1]
        if '.' not in nextPart:
            if 'mongo' in pathElements:
                expectedUnified = 'catapult:mongo:' + nextPart + ':'
            else:
                expectedUnified = 'catapult:' + nextPart + ':'
        elif pathElements[-1] in ['constants.h', 'types.h']:
            expectedUnified = 'catapult:'

        return nsUnified == expectedUnified

    @staticmethod
    def firstIncludeCheck(pathElements):
        fullPath = '/'.join(pathElements)
        if fullPath in PLUGINS_FIRSTINCLUDES:
            return '"{}"'.format(PLUGINS_FIRSTINCLUDES[fullPath])
        elif 'validators' in pathElements:
            return '"Validators.h"'
        elif 'observers' in pathElements:
            return '"Observers.h"'
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(originalIncludes, pathElements):
        del originalIncludes
        if 'tests' in pathElements and pathElements[-1].endswith('Tests.cpp'):
            fullPath = '/'.join(pathElements)
            if fullPath in PLUGINS_FIRSTINCLUDES:
                return '"{}"'.format(PLUGINS_FIRSTINCLUDES[fullPath])
            elif 'validators' in pathElements:
                return '"src/validators/Validators.h"'
            elif 'observers' in pathElements:
                return '"src/observers/Observers.h"'

            testsId = pathElements.index('tests')
            if 'int' in pathElements and pathElements.index('int') == testsId + 1:
                testsId += 1

            includeDir = '/'.join(pathElements[testsId + 1 : -1])
            includeFilePath = '/'.join(filter(None, [includeDir, pathElements[-1][:-9]]))
            return '"src/{}.h"'.format(includeFilePath)
        elif 'test' in pathElements:
            return '"{}.h"'.format(pathElements[-1][:-4])

        return '<could not figure out first include>'

class ToolsRules:
    @staticmethod
    def namespaceCheck(nsUnified, fullPath):
        splittedPath = re.split(r'[/\\]', fullPath)
        pathUnified = ':'.join(splittedPath)
        if nsUnified.startswith('catapult:tools'):
            nsUnified = nsUnified[len('catapult:'):]
        return nsUnified in pathUnified

    @staticmethod
    def firstIncludeCheck(pathElements):
        fullPath = '/'.join(pathElements)
        if fullPath in TOOLS_FIRSTINCLUDES:
            return '"{}"'.format(TOOLS_FIRSTINCLUDES[fullPath])
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(originalIncludes, pathElements):
        del originalIncludes
        del pathElements
        raise 'firstTestIncludeCheck called for a tool'

FILTER_NAMESPACES = (
    re.compile(r'.*detail'),
    re.compile(r'.*_types::'),
    re.compile(r'.*_types$'),
)

RULE_ID_TO_CLASS_MAP = {
    Rules.Default: DefaultRules,
    Rules.Plugin: PluginRules,
    Rules.Tools: ToolsRules
}

SOURCE_DIRS = dict((k, RULE_ID_TO_CLASS_MAP[v]) for k, v in USER_SOURCE_DIRS.items())

def isSpecialInclude(includePath):
    for filtered in SPECIAL_INCLUDES:
        if re.match(filtered, includePath):
            return True
    return False

class CheckResult(Enum):
    Success = 1
    Multiple = 2
    Invalid = 3
    Empty = 4

class Group(Enum):
    Own = 1
    Libs = 2
    System = 3

def checkExternalInclude(incA, incB):
    extA = incA.startswith('<ref10') or incA.startswith('<ripemd160') or incA.startswith('<sha3')
    extB = incB.startswith('<ref10') or incB.startswith('<ripemd160') or incB.startswith('<sha3')
    if extA and not extB:
        return True
    elif not extA and extB:
        return False
    return None

def checkCppInclude(incA, incB):
    boostA = incA.startswith('<boost') or incA.startswith('<mongocxx') or incA.startswith('<bsoncxx')
    boostB = incB.startswith('<boost') or incB.startswith('<mongocxx') or incB.startswith('<bsoncxx')
    if boostA and not boostB:
        return True
    elif not boostA and boostB:
        return False

INCLUDE_PRIORITIES = {'"src': 100, '"plugins': 150, '"catapult': 200, '"tests': 300}
def checkLocalInclude(partA, partB):
    valA = INCLUDE_PRIORITIES[partA] if partA in INCLUDE_PRIORITIES else 1
    valB = INCLUDE_PRIORITIES[partB] if partB in INCLUDE_PRIORITIES else 1
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

    if len(pathA) > 2 and len(pathA) == 2:
        return True

    return None

class SortableInclude(HeaderParser.Include):
    def __init__(self, inc, ruleset):
        super().__init__(inc.line, inc.lineno, inc.include, inc.rest)
        self.ruleset = ruleset

        includeStr = self.include
        if includeStr.startswith('"'):
            self.group = Group.Own
        elif SortableInclude.isLibraryInclude(includeStr):
            self.group = Group.Libs
        else:
            self.group = Group.System

    @staticmethod
    def isLibraryInclude(name):
        for libraryName in ['<ref10', '<ripemd160', '<sha3', '<boost']:
            if name.startswith(libraryName):
                return True
        return False

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
            result = checkLocalInclude(pathA[0], pathB[0])
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
            selfCHeader = self.include.endswith('.h>')
            otherCHeader = other.include.endswith('.h>')
            if selfCHeader and not otherCHeader:
                return False
            if (selfCHeader and otherCHeader) or not (selfCHeader or otherCHeader):
                return self.comparePaths(other)
            return True
        else:
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
        splitted = re.split(r'[/\\]', self.fullPath())
        if splitted[0] == 'tests':
            self.own = '/'.join(splitted[:-1]) + '/'
        else:
            self.own = '/'.join(splitted[1:-1]) + '/'

    def fullPath(self):
        return os.path.join(self.path, self.filename)

    def setIncludes(self, includes):
        self.includes = includes

    def setNamespaces(self, namespaces):
        self.namespaces = namespaces

    def setTemplateErrors(self, templateErrors):
        self.templateErrors = templateErrors

    def check(self):
        for filtered in NAMESPACES_FALSEPOSITIVES:
            if re.match(filtered, self.fullPath()):
                return CheckResult.Success

        if not self.namespaces:
            return CheckResult.Empty

        if len(self.namespaces) != 1:
            return CheckResult.Multiple

        namespace = self.namespaces[0].name.split('::')
        namespace.append('')
        nsUnified = ':'.join(namespace)
        if self.ruleset.namespaceCheck(nsUnified, self.fullPath()):
            return CheckResult.Success
        return CheckResult.Invalid

    def fixRelative(self, elem):
        temp = re.sub(self.own, '', elem.include)
        elem.include = temp
        #if '/' not in temp: elem.include = temp
        temp = re.sub(self.own, '', elem.line)
        elem.line = temp
        #if '/' not in temp: elem.line = temp

    def checkIncludes(self, errorReporter, preprocessor):
        toSort = []
        originalIncludes = []
        for elem in preprocessor:
            if elem.type != HeaderParser.PpType.Include:
                continue

            if isSpecialInclude(elem.include):
                continue

            #self.fixRelative(elem)
            originalIncludes.append(SortableInclude(elem, self.ruleset))
            toSort.append(SortableInclude(elem, self.ruleset))

        fullPath = self.fullPath()
        #print('----- %s sorting %d includes' %(fullPath, len(toSort)))
        #print(' presort', list(map(str, toSort)))
        toSort.sort()
        #print('  sorted', list(map(str, toSort)))

        # move "own" header to first position (i.e. RemoteChainApi.cpp including RemoteChainApi.h)
        pathElements = re.split(r'[/\\]', fullPath)
        if fullPath.endswith('.cpp'):
            ownHeader = '<unknown>'
            if 'src' in pathElements or 'tools' in pathElements:
                ownHeader = self.ruleset.firstIncludeCheck(pathElements)
            elif 'tests' in pathElements:
                ownHeader = self.ruleset.firstTestIncludeCheck(originalIncludes, pathElements)
            for i, elem in enumerate(toSort):
                if ownHeader == elem.include:
                    toSort.insert(0, toSort.pop(i))
                    break
            if ownHeader != originalIncludes[0].include:
                errorReporter('firstInclude', FirstIncludeError(fullPath, ownHeader, originalIncludes[0].include))

        if originalIncludes != toSort:
            print('I got includes mismatch')
            print('original', list(map(str, originalIncludes)))
            print('     new', list(map(str, toSort)))
            errorReporter('includesOrder', IncludesError(fullPath, toSort))
        return None

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
                if re.match(filtered, err.path):
                    skip = True
                    break
        elif 'tooLongLines' == groupName:
            for filtered in LONGLINES_FALSEPOSITIVES:
                if re.match(filtered, name):
                    skip = True
                    break
        if not skip:
            self.xml(groupName, err)

class ConReporter:
    def __init__(self):
        pass

    @staticmethod
    def formatFailure(buff, kind, name, msg):
        del kind, name
        buff.write(msg)

    def header(self, testsCount, failuresCount):
        pass

    def suite(self, suiteName, overallCount, errors): # pylint: disable=no-self-use
        print('===== {} ===== (tests: {}, failures: {})'.format(suiteName, overallCount, len(errors)))
        for err in errors:
            print(err)
        print('')

    def footer(self):
        pass

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
    def __init__(self):
        self.includes = {}
        self.context = AutoContainer()
        self.conReporter = ConReporter()
        self.simpleValidators = None
        self.presentExclusions = defaultdict(set)

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

    def add(self, entry):
        path = entry.fullPath()
        self.validateMaps(path)

        for skipFile in SKIP_FILES:
            if re.match(skipFile, path):
                return
        self.includes[path] = entry
        errorReporter = FilteredReporter(self.context)
        self.simpleValidators = validation.createValidators(errorReporter)
        headers = HeaderParser.HeaderParser(errorReporter, path, self.simpleValidators, fixIndentsInFiles=FIX_INDENTS)

        entry.setIncludes(filterNonProjectIncludes(headers.includes))
        entry.checkIncludes(errorReporter, headers.preprocessor)

        namespaces = Parser.NamespacesParser(path)
        namespaceCandidates = self.getShortestNamespaceSet(namespaces)

        entry.setNamespaces(namespaceCandidates)
        entry.setTemplateErrors(namespaces.templateErrors)

    def printFormatting(self):
        if TEXT_OUTPUT:
            self.printFormattingOut(self.conReporter)
            return

        with open('_build/tests.formatting2.xml', 'w') as outputFile:
            reporter = XmlReporter(outputFile)
            self.printFormattingOut(reporter)

    def getExclusionErrors(self, reporter):
        exclusionErrors = []
        for exclusionName, exclusionMap in EXCLUSIONS.items():
            if len(self.presentExclusions[exclusionName]) == len(exclusionMap):
                continue

            for missingExcl in exclusionMap:
                if missingExcl in self.presentExclusions[exclusionName]:
                    continue

                with io.StringIO() as output:

                    name = 'scripts/lint/exclutions.py'
                    msg = '{}: Exclusion from set: {}: >>{}<< was never hit'.format(name, exclusionName, missingExcl)
                    reporter.formatFailure(output, 'exclusions', name, msg)
                    exclusionErrors.append(output.getvalue())
        return exclusionErrors


    def printFormattingOut(self, reporter):
        formatted = AutoContainer()

        consecutives = []
        indents = []
        emptyLinesNearEnd = []
        firstInclude = []
        includesOrderErrors = []

        for validator in self.simpleValidators:
            for err in self.context[validator.NAME]:
                with io.StringIO() as output:
                    name = err.path
                    reporter.formatFailure(output, validator.NAME, name, validator.formatError(err))
                    formatted[validator.NAME].append(output.getvalue())

        exclusionErrors = self.getExclusionErrors(reporter)

        for err in self.context['consecutiveEmpty']:
            with io.StringIO() as output:
                name = err.path
                msg = '{}:{} Consecutive empty lines: {}'.format(name, err.lineno, err.line)
                reporter.formatFailure(output, 'consecutive', name, msg)
                consecutives.append(output.getvalue())
        for err in self.context['indentedPreprocessor']:
            with io.StringIO() as output:
                name = err.path
                msg = '{}:{} Invalid indent, {}: {}'.format(name, err.lineno, err.kind, err.line)
                reporter.formatFailure(output, 'indent', name, msg)
                indents.append(output.getvalue())
        for err in self.context['emptyNearEnd']:
            with io.StringIO() as output:
                name = err.path
                msg = '{}:{} Empty line near end of file: >>{}<<'.format(name, err.lineno, err.line)
                reporter.formatFailure(output, 'emptyline', name, msg)
                emptyLinesNearEnd.append(output.getvalue())
        for err in self.context['firstInclude']:
            with io.StringIO() as output:
                name = err.path
                msg = '{} Expected first include to be: >>{}<<'.format(name, err.include)
                reporter.formatFailure(output, 'firstInclude', name, msg)
                firstInclude.append(output.getvalue())
        for err in self.context['includesOrder']:
            with io.StringIO() as output:
                name = err.path
                msg = '{} Includes needs fixing, proper order: >>{}<<'.format(name, '\n'.join(map(str, err.includes)))
                reporter.formatFailure(output, 'includes', name, msg)
                includesOrderErrors.append(output.getvalue())

        failuresCount = len(indents) + len(emptyLinesNearEnd) + len(includesOrderErrors)
        for validator in self.simpleValidators:
            failuresCount += len(formatted[validator.NAME])

        reporter.header(len(self.includes), failuresCount)

        for validator in self.simpleValidators:
            reporter.suite(validator.SUITE_NAME, len(self.includes), formatted[validator.NAME])

        reporter.suite('Exclusions', len(self.includes), exclusionErrors)
        reporter.suite('ConsecutiveEmpty', len(self.includes), consecutives)
        reporter.suite('Indents', len(self.includes), indents)
        reporter.suite('EmptyLinesNearEnd', len(self.includes), emptyLinesNearEnd)
        reporter.suite('FirstInclude', len(self.includes), firstInclude)
        reporter.suite('Includes', len(self.includes), includesOrderErrors)
        reporter.footer()

    def filter(self, expectedResult):
        return filter(lambda elem: elem[1].check() == expectedResult, sorted(self.includes.items()))

    def printNamespaceErrorsOut(self, reporter):
        inconsistent = []
        multiple = []
        other = []
        for _, namespace in self.filter(CheckResult.Invalid):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} namespace is inconsistent with file location: >>{}<<'.format(name, namespace.namespaces[0].name)
                reporter.formatFailure(output, 'inconsistent', name, msg)
                inconsistent.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.Multiple):
            with io.StringIO() as output:
                name = os.path.join(namespace.path, namespace.filename)
                msg = '{} Multiple: >>{}<<'.format(name, namespace.namespaces)
                reporter.formatFailure(output, 'multiple', name, msg)
                multiple.append(output.getvalue())
        for _, namespace in self.filter(CheckResult.Empty):
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
        if TEXT_OUTPUT:
            self.printNamespaceErrorsOut(self.conReporter)
        else:
            with open('_build/tests.namespaces.xml', 'w') as outputFile:
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
        if TEXT_OUTPUT:
            self.printTemplateErrorsOut(self.conReporter)
        else:
            with open('_build/tests.templates.xml', 'w') as outputFile:
                reporter = XmlReporter(outputFile)
                self.printTemplateErrorsOut(reporter)

    def deps(self):
        print('---- ---- ----')
        print('Dependency map')
        deps = defaultdict(set)
        for name, entry in sorted(self.includes.items()):
            temp = os.path.dirname(name)
            pathA = '::'.join(re.split(r'[/\\]', temp)[1:])
            included = list(map(os.path.dirname, filter(lambda p: 'catapult' in p, entry.includes)))
            if not included:
                continue
            for include in included:
                pathB = '::'.join(re.split(r'[/\\]', include))
                deps[pathA].add(pathB)
        for name, entries in deps.items():
            for entry in entries:
                if entry != 'catapult':
                    print('"{}" -> "{}"'.format(name, entry))


def main():
    colorama.init()

    for sourceDir in SOURCE_DIRS:
        if not os.path.exists(sourceDir):
            os.chdir('..')
            os.chdir('..')
            if not os.path.exists(sourceDir):
                raise IOError('could not find `{}` directory'.format(sourceDir))

    analyzer = Analyzer()
    for sourceDir, ruleset in SOURCE_DIRS.items():
        for root, _, files in os.walk(sourceDir):
            for filename in files:
                if filename.endswith('.h') or filename.endswith('.cpp'):
                    print('parsing', root, filename)
                    analyzer.add(Entry(root, filename, ruleset))

    analyzer.printFormatting()
    analyzer.printNamespaces()
    analyzer.printTemplateErrors()

if __name__ == '__main__':
    main()
