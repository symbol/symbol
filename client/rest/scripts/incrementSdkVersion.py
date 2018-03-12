import codecs
import json
import os
import re
from collections import OrderedDict


def increment_version_string(version_string):
    match = re.match(r'(?P<major>.*)\.(?P<minor>.*)\.(?P<patch>.*)', version_string)
    return '{0}.{1}.{2}'.format(match.group('major'), int(match.group('minor')) + 1, match.group('patch'))


def mutate_json_data(filename, mutator):
    with codecs.open(filename, 'r', 'utf8') as input_file:
        data = json.load(input_file, object_pairs_hook=OrderedDict)

    mutator(data)

    with codecs.open(filename, 'w', 'utf8') as output_file:
        json.dump(data, output_file, indent=2)
        output_file.write('\n')


def increment_self_package_version(package_name):
    print('processing {0} (self) ...'.format(package_name))

    def mutator(data):
        data["version"] = increment_version_string(data["version"])

    mutate_json_data(os.path.join(package_name, "package.json"), mutator)
    mutate_json_data(os.path.join(package_name, "package-lock.json"), mutator)


def increment_dependent_package_version(package_name, dependency_package_name):
    print('processing {0} (dependent) ...'.format(package_name))

    def package_mutator(data):
        version = data["dependencies"][dependency_package_name]
        data["dependencies"][dependency_package_name] = increment_version_string(version)

    def package_lock_mutator(data):
        version = data["dependencies"][dependency_package_name]["version"]
        data["dependencies"][dependency_package_name]["version"] = increment_version_string(version)

    mutate_json_data(os.path.join(package_name, "package.json"), package_mutator)
    mutate_json_data(os.path.join(package_name, "package-lock.json"), package_lock_mutator)


def main():
    increment_self_package_version("catapult-sdk")
    for package_name in ["monitor", "rest", "spammer"]:
        increment_dependent_package_version(package_name, "catapult-sdk")


if __name__ == '__main__':
    main()
