import os
import shutil
from pathlib import Path


def rm_failure_handler(func, path, excinfo):
	del func
	del path
	if excinfo[0] != FileNotFoundError:
		raise excinfo[1]


class EnvironmentManager:
	def __init__(self, dry_run=False):
		self.dry_run = dry_run

	# region environment variables

	@property
	def system_bin_path(self):
		if self.dry_run:
			return '<SYSTEM_BIN_PATH>'

		for descriptor in [('ubuntu', '/usr/lib/x86_64-linux-gnu'), ('fedora', '/usr/lib64')]:
			if Path(descriptor[1]).exists():
				self._print_command('system_bin_path', ['detected', descriptor[1], 'for', descriptor[0]])
				return descriptor[1]

		raise RuntimeError('unable to detect system bin path')

	def set_env_var(self, key, value):
		self._print_command('set_env_var', [key, value])

		if self.dry_run:
			return

		os.environ[key] = value

	# endregion

	# region folder management

	def mkdirs(self, path, exist_ok=False):
		self._print_command('mkdirs', [path, exist_ok])

		if self.dry_run:
			return

		os.makedirs(path, exist_ok=exist_ok)

	def chdir(self, path):
		self._print_command('chdir', [path])

		if self.dry_run:
			return

		os.chdir(path)

	def rmtree(self, path):
		self._print_command('rmtree', [path])

		if self.dry_run:
			return

		shutil.rmtree(path, onerror=rm_failure_handler)

	# endregion

	# region globs

	def find_glob(self, directory_path, pattern, recursive=False):
		self._print_command('find_glob', [directory_path, pattern, recursive])

		if self.dry_run:
			return [f'[({pattern})match{i}]' for i in range(1, 3)]

		glob_func = Path.glob if not recursive else Path.rglob
		return glob_func(Path(directory_path), pattern)

	# endregion

	# region file copying

	def copy_glob_with_symlinks(self, directory_path, pattern, destination):
		self._print_command('copy_glob', [directory_path, pattern, destination])

		if self.dry_run:
			return

		for file in Path(directory_path).glob(pattern):
			shutil.copy(file, destination, follow_symlinks=False)

	def copy_tree_with_symlinks(self, source, destination):
		self._print_command('copy_tree', [source, destination])

		if self.dry_run:
			return

		shutil.copytree(source, destination, symlinks=True)

	def copy_with_symlink(self, source, destination):
		self._print_command('copy_tree', [source, destination])

		if self.dry_run:
			return

		shutil.copy(source, destination, follow_symlinks=False)

	# endregion

	# region file moving

	def move_glob_with_symlinks(self, directory_path, pattern, destination):
		self._print_command('move_glob', [directory_path, pattern, destination])

		if self.dry_run:
			return

		for file in Path(directory_path).glob(pattern):
			shutil.move(str(file), str(destination))

	# endregion

	@staticmethod
	def _print_command(command, args):
		formatted_args = ' '.join(str(x) for x in args)
		print(f'\033[0;33m[*] <{command}> {formatted_args}\033[0;0m')
