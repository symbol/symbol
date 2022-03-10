import argparse
import re
import sys
from pathlib import Path

from environment import EnvironmentManager
from process import ProcessManager

LINTER_DIR = Path('/linters')


def print_linter_status(name, return_code):
	return_code_description = 'succeeded' if not return_code else 'FAILED'
	print(f'{name} {return_code_description}')


def find_files_with_extension(environment_manager, extension):
	return list(environment_manager.find_glob(Path('.').resolve(), '*' + extension, recursive=True))


def run_cpp_linters(process_manager, dest_dir):
	cpp_lint_args = ['python3', str(LINTER_DIR.joinpath('cpp/checkProjectStructure.py').resolve())]
	for directory in ['src', 'extensions', 'plugins']:
		cpp_lint_args.extend(['--dep-check-dir', directory])

	cpp_lint_args.extend(['--dest-dir', dest_dir])

	process_manager.dispatch_subprocess(cpp_lint_args)


class LinterRunner:
	def __init__(self, process_manager, dest_dir, dry_run):
		self.process_manager = process_manager
		self.dest_dir = dest_dir
		self.dry_run = dry_run

		self.scope = ''
		self.output_filepath = ''

	def set_scope(self, scope):
		self.scope = scope
		self.output_filepath = Path(self.dest_dir) / f'{self.scope}.log'

	def run(self, args):
		linter_result = self.process_manager.dispatch_subprocess(args, handle_error=False, redirect_filename=self.output_filepath)
		print_linter_status(self.scope, linter_result)

	def fixup(self, modifier):
		if self.dry_run:
			return

		with open(self.output_filepath, 'rt', encoding='utf8') as infile:
			contents = infile.read()

		with open(self.output_filepath, 'wt', encoding='utf8') as outfile:
			for line in contents.split('\n'):
				line = modifier(line)
				outfile.write(f'{line}\n')

	def cat(self):
		self.process_manager.dispatch_subprocess(['cat', self.output_filepath])


def run_shell_linters(linter_runner, shell_files):
	linter_runner.set_scope('shellcheck')
	linter_runner.run(['shellcheck', '--format=gcc'] + shell_files)


def run_python_linters(linter_runner, python_files, script_path):
	pylint_warning_pattern = re.compile('[a-zA-Z\\-/]+\\.py:\\d+')

	linter_runner.set_scope('pylint')
	linter_runner.run([
		'pylint',
		'--rcfile', str(LINTER_DIR.joinpath('python/.pylintrc').resolve()),
		'--load-plugins', 'pylint_quotes',
		'--output-format', 'parseable'
	] + python_files)
	linter_runner.fixup(lambda line: line if not pylint_warning_pattern.match(line) else f'{script_path}/{line}')

	linter_runner.set_scope('pycodestyle')
	linter_runner.run(['pycodestyle', '--config', str(LINTER_DIR.joinpath('python/.pycodestyle').resolve())] + python_files)

	isort_warning_pattern = re.compile('([A-Z]+): ([a-zA-Z\\-/]+\\.py) (.*)')

	linter_runner.set_scope('isort')
	linter_runner.run(['isort', '--check-only', '--line-length', '140'] + python_files)
	linter_runner.fixup(lambda line: isort_warning_pattern.sub(
		lambda match: f'{match.group(2)}:1:1 {match.group(1).lower()}: {match.group(3)}',
		line))


def get_script_path():
	return Path(sys.argv[0]).resolve().parent


def main():
	parser = argparse.ArgumentParser(description='catapult lint runner')
	parser.add_argument('--out-dir', help='directory in which to store lint output files', required=True)
	parser.add_argument('--dry-run', help='outputs desired commands without running them', action='store_true')
	args = parser.parse_args()

	process_manager = ProcessManager(args.dry_run)
	environment_manager = EnvironmentManager(args.dry_run)
	environment_manager.set_env_var('HOME', '/tmp')

	run_cpp_linters(process_manager, args.out_dir)

	script_path = get_script_path()
	environment_manager.chdir(script_path)

	linter_runner = LinterRunner(process_manager, args.out_dir, args.dry_run)
	run_shell_linters(linter_runner, find_files_with_extension(environment_manager, '.sh'))
	run_python_linters(linter_runner, find_files_with_extension(environment_manager, '.py'), str(script_path))


if __name__ == '__main__':
	main()
