import sys
from pathlib import Path
from subprocess import PIPE, STDOUT, Popen, SubprocessError

MAX_LINE_LENGTH = 140


class ProcessManager:
	def __init__(self, dry_run=False):
		self.dry_run = dry_run

	def dispatch_subprocess(self, command_line, show_output=True, handle_error=True, redirect_filename=None):
		self._print_command(command_line)

		if self.dry_run:
			return 0

		with Popen(command_line, stdout=PIPE, stderr=STDOUT) as process:
			process_lines = []
			for line_bin in iter(process.stdout.readline, b''):
				line = line_bin.decode('utf-8')

				if show_output:
					sys.stdout.write(line)
					sys.stdout.flush()

				process_lines.append(line)

			process.wait()

			if redirect_filename:
				with open(redirect_filename, 'wt', encoding='utf8') as outfile:
					for line in process_lines:
						outfile.write(line)

			if handle_error:
				if 0 != process.returncode:
					formatted_command_line = ' '.join(map(str, command_line))
					raise SubprocessError(f'{formatted_command_line} exited with {process.returncode}')
			else:
				return process.returncode

		return 0

	def dispatch_test_subprocess(self, command_line, verbosity):
		self._print_command(command_line)

		if self.dry_run:
			return 0

		with Popen(command_line, stdout=PIPE, stderr=STDOUT) as process:
			process_lines = []
			is_filtered_output = 'max' != verbosity
			for line_bin in iter(process.stdout.readline, b''):
				line = line_bin.decode('utf-8')
				process_lines.append(line)

				if is_filtered_output:
					if not line.startswith('['):
						continue

					if 'suite' == verbosity and any(line.startswith(prefix) for prefix in ('[ RUN      ]', '[       OK ]')):
						continue

				sys.stdout.write(line)
				sys.stdout.flush()

			process.wait()

			if is_filtered_output and 0 != process.returncode:
				for line in process_lines:
					sys.stdout.write(line)
					sys.stdout.flush()

			print(f'process exited with return code: {process.returncode}')
			return process.returncode

	@staticmethod
	def _print_command(command_line):
		full_command_line = ' '.join(map(str, command_line))
		if MAX_LINE_LENGTH > len(full_command_line):
			ProcessManager._print_info_line('[*]', full_command_line)
		else:
			ProcessManager._print_info_line('[*]', command_line[0])
			for part in command_line[1:]:
				ProcessManager._print_info_line('     ', part)

	@staticmethod
	def _print_info_line(prefix, line):
		print(f'\033[0;36m{prefix} {line}\033[0;0m')

	def list_dir(self, path):
		command = ['ls', '-laF'] if 'win32' != sys.platform else ['cmd', '/c', 'dir']
		command.extend([Path(path).resolve()])
		self.dispatch_subprocess(command)
