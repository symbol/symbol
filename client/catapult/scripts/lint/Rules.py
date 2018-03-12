import re
from enum import Enum

from exclusions import \
    CORE_FIRSTINCLUDES, \
    EXTENSION_FIRSTINCLUDES, \
    PLUGINS_FIRSTINCLUDES, \
    TOOLS_FIRSTINCLUDES

class Rules(Enum):
    Default = 1
    Plugin = 2
    Extension = 3
    Tools = 4

class DefaultRules:
    @staticmethod
    def checkCatapultTest(pathUnified, splittedPath):
        if 'tests:test:' in pathUnified:
            return True
        if 'tests' in splittedPath and 'test' in splittedPath:
            return True
        # hack for TestHarness.h, as it's the only file that's directly inside tests/ directory,
        # but it has namespace catapult::test
        if 'tests:TestHarness.h' == pathUnified:
            return True
        return False

    @staticmethod
    def namespaceCheck(nsUnified, fullPath):
        # ignore trailing '_foo' as part of namespace
        splittedPath = re.split(r'[/\\]', fullPath)
        splittedPath[:] = [part if part.rfind('_') < 0 else part[0:part.rfind('_')] for part in splittedPath]

        pathUnified = ':'.join(splittedPath)
        if nsUnified == 'catapult:test:':
            return DefaultRules.checkCatapultTest(pathUnified, splittedPath)
        elif nsUnified == 'catapult:mocks:':
            nsUnified = ':mocks:'
            if re.search(r':Mock', pathUnified):
                return True

        # if there is no 'catapult:test' in a path, but path looks like test
        if 'tests' in splittedPath and 'test' in splittedPath:
            return False

        return nsUnified in pathUnified

    @staticmethod
    def firstIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
        fullPath = '/'.join(pathElements)
        if fullPath in CORE_FIRSTINCLUDES:
            return '"{}"'.format(CORE_FIRSTINCLUDES[fullPath])
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(sortedIncludes, pathElements):
        fullPath = '/'.join(pathElements)
        if 'int' in pathElements:
            return sortedIncludes[0].include

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

        nextPart = pathElements[elementId + 1]
        if 'test' == nextPart and 'mocks' in pathElements:
            nextPart = 'mocks' # use mocks as appropriate

        if '.' not in nextPart:
            expectedUnified = 'catapult:' + nextPart + ':'
            if 'test' in pathElements:
                expectedUnified = 'catapult:test:'
                if 'mocks' in pathElements:
                    expectedUnified = 'catapult:mocks:'
        elif pathElements[-1] in ['constants.h', 'types.h']:
            expectedUnified = 'catapult:'

        return nsUnified == expectedUnified

    @staticmethod
    def firstIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
        fullPath = '/'.join(pathElements)
        if fullPath in PLUGINS_FIRSTINCLUDES:
            return '"{}"'.format(PLUGINS_FIRSTINCLUDES[fullPath])
        elif 'validators' in pathElements:
            return '"Validators.h"'
        elif 'observers' in pathElements:
            return '"Observers.h"'
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
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

class ExtensionRules:
    @staticmethod
    def namespaceCheck(nsUnified, fullPath):
        pathElements = re.split(r'[/\\]', fullPath)
        expectedUnified = 'catapult:' + pathElements[1] + ':'

        elementId = None
        for root in ['src', 'int', 'tests']:
            if root in pathElements:
                elementId = pathElements.index(root)
                break

        if elementId is None:
            # hack: extension entry points should have anon namespaces with impls
            if pathElements[-1].endswith('Extension.cpp'):
                expectedUnified += '<anon>:'

            return nsUnified == expectedUnified, expectedUnified

        # determine expected namespace
        nextPart = pathElements[elementId + 1]
        if '.' not in nextPart:
            expectedUnified += nextPart + ':'

        # overrides
        if 'test' in pathElements:
            expectedUnified = 'catapult:test:'
            if 'mocks' in pathElements:
                expectedUnified = 'catapult:mocks:'
        elif 'plugins' in pathElements:
            expectedUnified = 'catapult:mongo:plugins:'
        else:
            for subcomponent in ['api', 'chain', 'handlers']:
                if subcomponent in pathElements:
                    expectedUnified = 'catapult:' + subcomponent + ':'
                    break

        return nsUnified == expectedUnified, expectedUnified

    @staticmethod
    def firstIncludeCheck(sortedIncludes, pathElements):
        fullPath = '/'.join(pathElements)
        if pathElements[-1].endswith('Extension.cpp'):
            return sortedIncludes[0].include
        if fullPath in EXTENSION_FIRSTINCLUDES:
            return '"{}"'.format(EXTENSION_FIRSTINCLUDES[fullPath])
        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
        if 'tests' in pathElements and pathElements[-1].endswith('Tests.cpp'):
            fullPath = '/'.join(pathElements)
            if fullPath in EXTENSION_FIRSTINCLUDES:
                return '"{}"'.format(EXTENSION_FIRSTINCLUDES[fullPath])

            testsId = pathElements.index('tests')
            if 'int' in pathElements and pathElements.index('int') == testsId + 1:
                testsId += 1

            includeDir = '/'.join(pathElements[testsId + 1 : -1])
            includeFilePath = '/'.join(filter(None, [includeDir, pathElements[-1][:-9]]))
            if 'plugins' in pathElements: # subplugins should be relative to the subplugin root
                return '"src/{}.h"'.format(includeFilePath)

            # non-plugins should be relative to the extension root
            return '"{}/src/{}.h"'.format(pathElements[1], includeFilePath)
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

        # hack for tools: main.cpp contains everything in anon namespace if that is the case
        # drop the anon part of the namespace
        if splittedPath[-1] == 'main.cpp':
            nsUnified = re.sub(r'<anon>:$', '', nsUnified)

        return nsUnified in pathUnified

    @staticmethod
    def firstIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
        fullPath = '/'.join(pathElements)
        if fullPath in TOOLS_FIRSTINCLUDES:
            return '"{}"'.format(TOOLS_FIRSTINCLUDES[fullPath])

        if 'main.cpp' == pathElements[-1]:
            return '"tools/ToolMain.h"'

        return '"{}.h"'.format(pathElements[-1][:-4])

    @staticmethod
    def firstTestIncludeCheck(sortedIncludes, pathElements):
        del sortedIncludes
        del pathElements
        raise 'firstTestIncludeCheck called for a tool'

RULE_ID_TO_CLASS_MAP = {
    Rules.Default: DefaultRules,
    Rules.Plugin: PluginRules,
    Rules.Extension: ExtensionRules,
    Rules.Tools: ToolsRules
}
