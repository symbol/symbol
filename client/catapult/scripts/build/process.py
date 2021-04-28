import sys
from subprocess import PIPE, STDOUT, Popen

MAX_LINE_LENGTH = 140


def dispatch_subprocess(command_line, show_output=True):
    process = Popen(command_line, stdout=PIPE, stderr=STDOUT)

    for line_bin in iter(process.stdout.readline, b''):
        line = line_bin.decode('utf-8')

        if show_output:
            sys.stdout.write(line)
            sys.stdout.flush()

    process.wait()

    return process.returncode


class ProcessManager:
    def __init__(self, dry_run=False):
        self.dry_run = dry_run

    def dispatch_subprocess(self, command_line, show_output=True):
        full_command_line = ' '.join(command_line)
        if MAX_LINE_LENGTH > len(full_command_line):
            self._print_info_line('[*]', full_command_line)
        else:
            self._print_info_line('[*]', command_line[0])
            for part in command_line[1:]:
                self._print_info_line('     ', part)

        if self.dry_run:
            return 0

        return dispatch_subprocess(command_line, show_output)

    @staticmethod
    def _print_info_line(prefix, line):
        print('\033[0;36m{} {}\033[0;0m'.format(prefix, line))
