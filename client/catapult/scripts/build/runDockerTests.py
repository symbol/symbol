import argparse
import sys
from pathlib import Path

from environment import EnvironmentManager
from process import ProcessManager

MONGO_DIR = Path('mongo')
OUTPUT_DIR = Path('catapult-data')


def get_image_label(image_name):
    return image_name[image_name.index(':') + 1:]


def create_docker_compose_command(mode):
    return [
        'docker-compose', 'up',
        '--no-color',
        '--abort-on-container-exit',
        '--exit-code-from', mode
    ]


def prepare_docker_compose_file(input_filepath, image_name, user, verbosity, outfile):
    replacements = [
        ('{{IMAGE_NAME}}', image_name),
        ('{{USER}}', '"{}"'.format(user)),

        ('{{BUILD_NUMBER}}', get_image_label(image_name)),
        ('{{NETWORK_IP}}', '3000'),
        ('{{GTESTFILTER}}', '*'),
        ('{{STRESSCOUNT}}', '1'),
        ('{{VERBOSITY}}', verbosity)
    ]

    with open(input_filepath, 'rt') as infile:
        contents = infile.read()
        for replacement in replacements:
            contents = contents.replace(replacement[0], replacement[1])

        outfile.write(contents)


def main():
    parser = argparse.ArgumentParser(description='catapult tests runner')
    parser.add_argument('--image', help='docker tests image', required=True)
    parser.add_argument('--user', help='docker user', required=True)
    parser.add_argument('--mode', help='test mode', choices=('bench', 'test'), required=True)
    parser.add_argument('--verbosity', help='verbosity level', default='max')
    parser.add_argument('--dry-run', help='outputs desired commands without runing them', action='store_true')
    args = parser.parse_args()

    process_manager = ProcessManager(args.dry_run)

    compose_template_directory = Path(__file__).parent / 'templates'
    compose_template_filepath = compose_template_directory / 'Run{}.yaml'.format(args.mode.capitalize())
    print('processing template from {}'.format(compose_template_filepath))
    prepare_args = [compose_template_filepath, args.image, args.user, args.verbosity]
    prepare_docker_compose_file(*(prepare_args + [sys.stdout]))  # pylint: disable=too-many-function-args

    if not args.dry_run:
        with open('docker-compose.yaml', 'wt') as outfile:
            prepare_docker_compose_file(*(prepare_args + [outfile]))

    environment_manager = EnvironmentManager(args.dry_run)
    environment_manager.rmtree(OUTPUT_DIR)
    environment_manager.mkdirs(OUTPUT_DIR)
    environment_manager.mkdirs(OUTPUT_DIR / 'logs')
    environment_manager.mkdirs(OUTPUT_DIR / 'workdir')

    environment_manager.rmtree(MONGO_DIR)
    environment_manager.mkdirs(MONGO_DIR / get_image_label(args.image))

    process_manager.dispatch_subprocess(['ls', '-laF'])
    process_manager.dispatch_subprocess(['ls', '-laF', 'catapult-src'])

    docker_compose_args = create_docker_compose_command(args.mode)
    if process_manager.dispatch_subprocess(docker_compose_args, handle_error=False):
        print('tests failed')
        sys.exit(1)

    print('tests succeeded')


if __name__ == '__main__':
    main()
