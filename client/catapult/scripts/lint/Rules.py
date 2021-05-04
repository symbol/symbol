import re
from enum import Enum

from exclusions import CORE_FIRSTINCLUDES, EXTENSION_FIRSTINCLUDES, PLUGINS_FIRSTINCLUDES, TOOLS_FIRSTINCLUDES


class Rules(Enum):
    DEFAULT = 1
    PLUGIN = 2
    EXTENSION = 3
    TOOLS = 4


def get_major_component_name(component):
    separator_index = component.find('_')
    if -1 == separator_index:
        return component

    return component[0:separator_index]


def split_path(full_path):
    return re.split(r'[/\\]', full_path)


def split_path_and_strip_underscores(full_path):  # pylint: disable=invalid-name
    # ignore trailing '_foo' as part of namespace
    splitted_path = split_path(full_path)
    return [part if part.rfind('_') < 0 else part[0:part.rfind('_')] for part in splitted_path]


class DefaultRules:
    @staticmethod
    def check_catapult_test(path_unified, splitted_path):
        if 'tests:test:' in path_unified:
            return True
        if 'tests' in splitted_path and 'test' in splitted_path:
            return True
        # hack for TestHarness.h, as it's the only file that's directly inside tests/ directory,
        # but it has namespace catapult::test
        if 'tests:TestHarness.h' == path_unified:
            return True
        return False

    @staticmethod
    def namespace_check(ns_unified, full_path):
        splitted_path = split_path_and_strip_underscores(full_path)

        path_unified = ':'.join(splitted_path)
        if ns_unified == 'catapult:test:':
            return DefaultRules.check_catapult_test(path_unified, splitted_path)

        if ns_unified == 'catapult:mocks:':
            ns_unified = ':mocks:'
            if re.search(r':Mock', path_unified):
                return True

        # if there is no 'catapult:test' in a path, but path looks like test
        if 'tests' in splitted_path and 'test' in splitted_path:
            return False

        # everything in int tests or bench, as there's no hierarchy there and we can't figure out ns
        if 'int' in splitted_path or 'bench' in splitted_path:
            return True

        return ns_unified in path_unified

    @staticmethod
    def first_include_check(sorted_includes, path_elements):
        del sorted_includes
        full_path = '/'.join(path_elements)
        if full_path in CORE_FIRSTINCLUDES:
            return '"{}"'.format(CORE_FIRSTINCLUDES[full_path])
        return '"{}.h"'.format(path_elements[-1][:-4])

    @staticmethod
    def first_test_include_check(sorted_includes, path_elements):
        full_path = '/'.join(path_elements)
        if 'test' not in path_elements:
            if 'int' in path_elements or 'bench' in path_elements:
                return sorted_includes[0].include

        if path_elements[0] != 'tests':
            return '<first_test_include_check called on non test path>' + full_path

        if full_path in CORE_FIRSTINCLUDES:
            include_path = CORE_FIRSTINCLUDES[full_path]
            if include_path[0] == '<':
                return include_path
            return '"{}"'.format(include_path)

        if path_elements[-1].endswith('Tests.cpp'):
            include_dir = '/'.join(path_elements[1:-1])
            return '"{}/{}.h"'.format(include_dir, path_elements[-1][:-9])
        return '"{}.h"'.format(path_elements[-1][:-4])

    @staticmethod
    def validate_cross_includes(sorted_includes, path_elements):
        invalid = []
        component = path_elements[2]
        for elem in sorted_includes:
            include_path_elements = elem.include[1:-1].split('/')
            if len(include_path_elements) <= 2:
                continue

            if path_elements[1] == 'catapult':
                if include_path_elements[1] == 'catapult' and include_path_elements[2] != component:
                    invalid.append(elem)

            if path_elements[1] == 'int':
                if include_path_elements[0] == 'tests' and include_path_elements[1] == 'catapult':
                    invalid.append(elem)

        return invalid


class PluginRules:
    @staticmethod
    def namespace_check(ns_unified, full_path):
        path_elements = split_path(full_path)
        if 'src' in path_elements:
            element_id = path_elements.index('src')
        elif 'int' in path_elements:
            element_id = path_elements.index('int')
        elif 'tests' in path_elements:
            element_id = path_elements.index('tests')
        else:
            return False

        expected_unified = 'catapult:plugins:'

        next_part = path_elements[element_id + 1]
        if 'test' == next_part and 'mocks' in path_elements:
            next_part = 'mocks'  # use mocks as appropriate

        if '.' not in next_part:
            expected_unified = 'catapult:' + next_part + ':'
            if 'test' in path_elements:
                expected_unified = 'catapult:test:'
                if 'mocks' in path_elements:
                    expected_unified = 'catapult:mocks:'
        elif path_elements[-1] in ['constants.h', 'types.h']:
            expected_unified = 'catapult:'

        return ns_unified == expected_unified

    @staticmethod
    def first_include_check(sorted_includes, path_elements):
        del sorted_includes
        full_path = '/'.join(path_elements)
        if full_path in PLUGINS_FIRSTINCLUDES:
            return '"{}"'.format(PLUGINS_FIRSTINCLUDES[full_path])

        if 'validators' in path_elements:
            return '"Validators.h"'

        if 'observers' in path_elements:
            return '"Observers.h"'

        return '"{}.h"'.format(path_elements[-1][:-4])

    @staticmethod
    def first_test_include_check(sorted_includes, path_elements):
        del sorted_includes
        if 'tests' in path_elements and path_elements[-1].endswith('Tests.cpp'):
            full_path = '/'.join(path_elements)
            if full_path in PLUGINS_FIRSTINCLUDES:
                return '"{}"'.format(PLUGINS_FIRSTINCLUDES[full_path])

            if 'validators' in path_elements:
                return '"src/validators/Validators.h"'

            if 'observers' in path_elements:
                return '"src/observers/Observers.h"'

            tests_id = path_elements.index('tests')
            if 'int' in path_elements and path_elements.index('int') == tests_id + 1:
                tests_id += 1

            include_dir = '/'.join(path_elements[tests_id + 1: -1])
            include_file_path = '/'.join(filter(None, [include_dir, path_elements[-1][:-9]]))
            return '"src/{}.h"'.format(include_file_path)

        if 'test' in path_elements:
            return '"{}.h"'.format(path_elements[-1][:-4])

        return '<could not figure out first include>'

    @staticmethod
    def validate_cross_includes(sorted_includes, path_elements):
        component = path_elements[2]

        invalid = []
        for elem in sorted_includes:
            include_path_elements = elem.include[1:-1].split('/')
            if len(include_path_elements) <= 2:
                continue

            if include_path_elements[0] != 'plugins':
                continue

            # allow cross inclusion of src like:
            # "plugins/txes/aggregate/src/model/AggregateEntityType.h"
            if 'tests' not in include_path_elements:
                continue

            # allow lock_hash to include lock_shared tests
            include_major_component = get_major_component_name(include_path_elements[2])
            file_major_component = get_major_component_name(component)
            if include_major_component != file_major_component:
                invalid.append(elem)

        return invalid


class ExtensionRules:
    @staticmethod
    def namespace_check(ns_unified, full_path):
        path_elements = split_path(full_path)
        expected_unified = 'catapult:' + path_elements[1] + ':'

        element_id = None
        for root in ['src', 'int', 'tests']:
            if root in path_elements:
                element_id = path_elements.index(root)
                break

        if element_id is None:
            # hack: extension entry points should have anon namespaces with impls
            if path_elements[-1].endswith('Extension.cpp'):
                expected_unified += '<anon>:'

            return ns_unified == expected_unified, expected_unified

        # determine expected namespace
        next_part = path_elements[element_id + 1]
        if '.' not in next_part:
            expected_unified += next_part + ':'

        # overrides
        if 'test' in path_elements:
            expected_unified = 'catapult:test:'
            if 'mocks' in path_elements:
                expected_unified = 'catapult:mocks:'
        elif 'plugins' in path_elements:
            expected_unified = 'catapult:mongo:plugins:'
        else:
            for subcomponent in ['api', 'chain', 'handlers', 'io', 'ionet', 'model']:
                if subcomponent in path_elements:
                    expected_unified = 'catapult:' + subcomponent + ':'
                    break

        return ns_unified == expected_unified, expected_unified

    @staticmethod
    def first_include_check(sorted_includes, path_elements):
        full_path = '/'.join(path_elements)
        if path_elements[-1].endswith('Extension.cpp'):
            return sorted_includes[0].include

        # exclusions have priority over general rules
        if full_path in EXTENSION_FIRSTINCLUDES:
            return '"{}"'.format(EXTENSION_FIRSTINCLUDES[full_path])

        if path_elements[-1].startswith('Mongo') and path_elements[-1].endswith('Plugin.cpp'):
            plugin_name = path_elements[-1][5:-10]
            return '"{}Mapper.h"'.format(plugin_name)

        if 'filters' in path_elements and 'timesync' in path_elements:
            return '"SynchronizationFilters.h"'

        return '"{}.h"'.format(path_elements[-1][:-4])

    @staticmethod
    def first_test_include_check(sorted_includes, path_elements):
        del sorted_includes
        if 'tests' in path_elements and path_elements[-1].endswith('Tests.cpp'):
            return ExtensionRules._first_test_include_check_for_tests_file(path_elements)

        if 'test' in path_elements:
            return '"{}.h"'.format(path_elements[-1][:-4])

        return '<could not figure out first include>'

    @staticmethod
    def _first_test_include_check_for_tests_file(path_elements):
        full_path = '/'.join(path_elements)
        if full_path in EXTENSION_FIRSTINCLUDES:
            return '"{}"'.format(EXTENSION_FIRSTINCLUDES[full_path])

        if path_elements[-1].startswith('Mongo') and path_elements[-1].endswith('PluginTests.cpp'):
            return '"mongo/tests/test/MongoPluginTestUtils.h"'

        if 'filters' in path_elements and 'timesync' in path_elements:
            return '"timesync/src/filters/SynchronizationFilters.h"'

        tests_id = path_elements.index('tests')
        if 'int' in path_elements and path_elements.index('int') == tests_id + 1:
            tests_id += 1

        include_dir = '/'.join(path_elements[tests_id + 1: -1])
        include_file_path = '/'.join(filter(None, [include_dir, path_elements[-1][:-9]]))
        if 'plugins' in path_elements:  # subplugins should be relative to the subplugin root
            return '"src/{}.h"'.format(include_file_path)

        # non-plugins should be relative to the extension root
        return '"{}/src/{}.h"'.format(path_elements[1], include_file_path)

    @staticmethod
    def validate_cross_includes(sorted_includes, path_elements):
        component = path_elements[1]

        invalid = []
        for elem in sorted_includes:
            include_path_elements = elem.include[1:-1].split('/')
            if include_path_elements[0] in ['tests', 'catapult']:
                continue

            # allow any non test like elements
            if 'tests' not in include_path_elements:
                continue

            # for mongo allow plugins inclusion inside extensions
            # extensions/mongo/plugins/<name> -> plugins/txes/<name>
            mongo_sub_component = get_major_component_name(path_elements[3])
            include_component = get_major_component_name(include_path_elements[2])
            if (component == 'mongo' and path_elements[2] == 'plugins'
                    and include_path_elements[0] == 'plugins' and include_path_elements[1] == 'txes'):
                if mongo_sub_component == include_component:
                    continue

            if include_path_elements[0] == component:
                if include_path_elements[0] != 'mongo':
                    continue

                # for mongo report if it's not `mongo/tests`
                if include_path_elements[1] == 'tests':
                    continue

            if mongo_sub_component != include_component:
                invalid.append(elem)

        return invalid


class ToolsRules:
    @staticmethod
    def namespace_check(ns_unified, full_path):
        splitted_path = split_path_and_strip_underscores(full_path)

        path_unified = ':'.join(splitted_path)
        if ns_unified.startswith('catapult:tools'):
            ns_unified = ns_unified[len('catapult:'):]

        # hack for tools: main.cpp contains everything in anon namespace if that is the case
        # drop the anon part of the namespace
        if splitted_path[-1] == 'main.cpp':
            ns_unified = re.sub(r'<anon>:$', '', ns_unified)

        return ns_unified in path_unified

    @staticmethod
    def first_include_check(sorted_includes, path_elements):
        del sorted_includes
        full_path = '/'.join(path_elements)
        if full_path in TOOLS_FIRSTINCLUDES:
            return '"{}"'.format(TOOLS_FIRSTINCLUDES[full_path])

        if 'main.cpp' == path_elements[-1]:
            return '"tools/ToolMain.h"'

        return '"{}.h"'.format(path_elements[-1][:-4])

    @staticmethod
    def first_test_include_check(sorted_includes, path_elements):
        del sorted_includes
        del path_elements
        raise 'first_test_include_check called for a tool'


RULE_ID_TO_CLASS_MAP = {
    Rules.DEFAULT: DefaultRules,
    Rules.PLUGIN: PluginRules,
    Rules.EXTENSION: ExtensionRules,
    Rules.TOOLS: ToolsRules
}
