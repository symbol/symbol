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

    # region file copying

    def copy_glob_with_symlinks(self, directory_path, pattern, destination):
        self._print_command('copy_glob', [pattern, destination])

        if self.dry_run:
            return

        for file in Path(directory_path).glob(pattern):
            shutil.copy(file, destination, follow_symlinks=False)

    def copy_tree_with_symlinks(self, source, destination):
        self._print_command('copy_tree', [source, destination])

        if self.dry_run:
            return

        shutil.copytree(source, destination, symlinks=True)

    # endregion

    @staticmethod
    def _print_command(command, args):
        print('\033[0;33m[*] <{}> {}\033[0;0m'.format(command, ' '.join(str(x) for x in args)))
