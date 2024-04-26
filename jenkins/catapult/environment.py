import os
import shutil
import sys
from pathlib import Path


def rm_onerror_handler(func, path, excinfo):
	rm_onexc_handler(func, path, excinfo[1])


def rm_onexc_handler(func, path, exception):
	del func
	del path
	if not isinstance(exception, FileNotFoundError):
		raise exception


class EnvironmentManager:
	def __init__(self, dry_run=False):
		self.dry_run = dry_run

	# region environment variables

	@property
	def system_bin_path(self):
		if self.dry_run:
			return '<SYSTEM_BIN_PATH>'

		for descriptor in [('ubuntu', '/usr/lib/x86_64-linux-gnu'), ('fedora', '/usr/lib64'), ('ubuntu arm64', '/usr/lib/aarch64-linux-gnu')]:
			if Path(descriptor[1]).exists():
				self._print_command('system_bin_path', ['detected', descriptor[1], 'for', descriptor[0]])
				return descriptor[1]

		raise RuntimeError('unable to detect system bin path')

	def get_env_var(self, key):
		self._print_command('get_env_var', [key])

		if self.dry_run:
			return f'env:{key}'

		return os.environ[key]

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

		kwargs = {'onexc': rm_onexc_handler} if sys.version_info >= (3, 12) else {'onerror': rm_onerror_handler}
		shutil.rmtree(path, **kwargs)

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

	def copy_glob_subtree_with_symlinks(self, directory_path, pattern, destination):
		self._print_command('copy_glob_tree', [directory_path, pattern, destination])

		if self.dry_run:
			return

		for file in Path(directory_path).rglob(pattern):
			shutil.copy(file, destination, follow_symlinks=False)

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

	@staticmethod
	def is_windows_platform():
		return 'win32' == sys.platform

	@staticmethod
	def root_directory(directory_name):
		root_directory_prefix = 'c:\\' if EnvironmentManager.is_windows_platform() else '/'
		return root_directory_prefix + directory_name
