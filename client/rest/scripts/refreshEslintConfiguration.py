from enum import Enum
import os
import shutil


TEMPLATE_DIRECTORY = os.path.join("scripts", "eslint-templates")


class Options(Enum):
    NONE = 0
    MONGO_SUPPORT = 1
    NO_TEST = 2


def mutate_file_lines(filename, mutator):
    with open(filename, 'r') as input_file:
        lines = input_file.readlines()

    mutator(lines)

    with open(filename, 'w') as output_file:
        output_file.writelines(lines)


def update_eslint_src(package_name, environments):
    dest_eslint_filename = os.path.join(package_name, ".eslintrc")
    shutil.copy(os.path.join(TEMPLATE_DIRECTORY, "src.eslintrc"), dest_eslint_filename)

    def mutator(lines):
        index = 1
        lines.insert(index, "env:\n")
        for environment in environments:
            index += 1
            lines.insert(index, "  {0}: true\n".format(environment))

    mutate_file_lines(dest_eslint_filename, mutator)


def update_eslint_test(package_name, options):
    dest_eslint_filename = os.path.join(package_name, "test", ".eslintrc")
    shutil.copy(os.path.join(TEMPLATE_DIRECTORY, "test.eslintrc"), dest_eslint_filename)

    if Options.MONGO_SUPPORT != options:
        return

    def mutator(lines):
        additional_mongo_rule_lines = [
            "",
            "  no-underscore-dangle:",
            "  - error",
            "  - allow:",
            "    - _id # mongodb identifier",
            "    - high_ # MongoDb.Timestamp",
            "    - low_ # MongoDb.Timestamp"
        ]

        for line in additional_mongo_rule_lines:
            lines.append('{0}\n'.format(line))

    mutate_file_lines(dest_eslint_filename, mutator)


def update_eslint(package_name, environments, options=Options.NONE):
    print('processing {0}...'.format(package_name))
    update_eslint_src(package_name, environments)
    if Options.NO_TEST != options:
        update_eslint_test(package_name, options)


def main():
    update_eslint("catapult-sdk", ["es6"])
    update_eslint("rest", ["es6", "node"], Options.MONGO_SUPPORT)
    update_eslint("tools", ["es6", "browser"], Options.NO_TEST)
    for package_name in ["monitor", "spammer"]:
        update_eslint(package_name, ["es6", "node"])


if __name__ == '__main__':
    main()
