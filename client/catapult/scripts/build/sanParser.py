import argparse
import re
import sys
from enum import Enum
from xml.sax.saxutils import quoteattr

# region io helpers


def _read_lines(filename):
	data = []
	with open(filename, encoding='utf8') as fin:
		for line in fin:
			data.append(line.rstrip())

	return data


def _output_xml_test_case(entry, outfile):
	outfile.write(f'''<testcase name="{entry.name}" status="run" time="0.002" classname="{entry.error}">
''')
	outfile.write(f'<failure message={quoteattr(entry.message)} type=""><![CDATA[')
	outfile.write(entry.create_header())
	outfile.write('\n'.join(entry.trace))
	outfile.write('>]]></failure>')
	outfile.write('</testcase>')


def _output_xml_to_file(parsed, sanitizer_name, outfile):
	outfile.write(f'''<?xml version="1.0" encoding="UTF-8"?>
<testsuites tests="1" failures="1" disabled="0" errors="0" timestamp="0" time="0" name="AllTests">
	<testsuite name="{sanitizer_name}" tests="1" failures="{len(parsed)}" disabled="0" errors="0" time="0">
''')
	for entry in parsed:
		_output_xml_test_case(entry, outfile)
	outfile.write('''
	</testsuite>
</testsuites>
''')


def _output_xml(parsed, sanitizer_name, filename):
	with open(filename, 'w', encoding='utf8') as outfile:
		_output_xml_to_file(parsed, sanitizer_name, outfile)


# endregion

# region SanError


class SanError:
	def __init__(self, pattern_summary, error_name_extractor):
		self.pattern_summary = re.compile(pattern_summary)
		self.error_name_extractor = error_name_extractor

		self.header = ''
		self.name = 'Invalid name'
		self.error = 'Invalid error'
		self.message = 'Invalid message'

		self.trace = []

	def parse(self, line):
		self.header = line
		match = self.pattern_summary.search(line)
		if match:
			(self.error, self.name) = self.error_name_extractor(match)
			self.message = f'{match.group(1)} {match.group(3)}'

	def create_header(self):
		return f'{self.header}\n'

# endregion

# region BasicParser


class BasicParser:
	def __init__(self, start_state, end_pattern, end_state):
		self.start_state = start_state
		self.end_pattern = end_pattern
		self.end_state = end_state

		self.state = self.start_state
		self.current_error = None
		self.parsed = []
		self.handlers = {}

	def push(self, line):
		self.handlers[self.state](line)

	def parse_details(self, line):
		if line.startswith(self.end_pattern):
			self.state = self.end_state
			self.current_error.parse(line)
			self.parsed.append(self.current_error)
		else:
			self.current_error.trace.append(line)

# endregion

# region AsanParser


class AsanParser(BasicParser):
	class ParserState(Enum):
		START_MARKER = 1
		CONTENTS = 10

	def __init__(self):
		super().__init__(self.ParserState.START_MARKER, 'SUMMARY', self.ParserState.START_MARKER)

		self.handlers = {
			self.ParserState.START_MARKER: self.parse_start_marker,
			self.ParserState.CONTENTS: self.parse_details
		}

	def parse_start_marker(self, line):
		if '==ERROR:' in line:
			self.current_error = self.create_asan_error()
			self.current_error.parse(line)

			self.state = self.ParserState.CONTENTS
		elif 'runtime error:' in line:
			self.current_error = self.create_ubsan_error()
			self.current_error.parse(line)

			self.state = self.ParserState.CONTENTS

	@staticmethod
	def create_asan_error():
		return SanError(r'==ERROR: ([^:]*)(: )(.*)', lambda match: ('sanitizer', match.group(1)))

	@staticmethod
	def create_ubsan_error():
		return SanError(r'^([^:]*):(\d+):\d+: (.*) 0x.*', lambda match: ('sanitizer', match.group(1)))

# endregion

# region TsanParser


class TsanParser(BasicParser):
	class ParserState(Enum):
		START_MARKER = 1
		WARNING = 2
		DETAILS = 3
		END_MARKER = 5

	def __init__(self):
		super().__init__(self.ParserState.START_MARKER, 'SUMMARY', self.ParserState.END_MARKER)

		self.handlers = {
			self.ParserState.START_MARKER: self.parse_start_header,
			self.ParserState.WARNING: self.create_checker('WARNING: ', self.ParserState.DETAILS),
			self.ParserState.DETAILS: self.parse_details,
			self.ParserState.END_MARKER: self.create_checker('==================', self.ParserState.START_MARKER)
		}

	def parse_start_header(self, line):
		if line.startswith('=================='):
			self.current_error = self.create_error()
			self.state = self.ParserState.WARNING

	def create_checker(self, expected_beginning, next_state):
		def checker(line):
			if not line.startswith(expected_beginning):
				print('INVALID MARKER')
				sys.exit(1)
			self.state = next_state

		return checker

	@staticmethod
	def create_error():
		return SanError(r'ThreadSanitizer: (.*) (\/.*|\(\/.*) (in .*)', lambda match: (match.group(1), match.group(2)))

# endregion

# region parse_san_log / main


def parse_san_log(input_filepath, output_filepath, mode):
	sanitizer_name = ''
	parser = None
	if 'asan' == mode:
		sanitizer_name = 'Address'
		parser = AsanParser()
	elif 'tsan' == mode:
		sanitizer_name = 'Tsan'
		parser = TsanParser()

	for line in _read_lines(input_filepath):
		parser.push(line)

	_output_xml(parser.parsed, sanitizer_name, output_filepath)


def main():
	parser = argparse.ArgumentParser(description='sanitizer log parser')
	parser.add_argument('--input', help='path to input file', required=True)
	parser.add_argument('--output', help='path to output file', required=True)
	parser.add_argument('--mode', help='parsing mode', choices=('asan', 'tsan'), required=True)
	args = parser.parse_args()

	parse_san_log(args.input, args.output, args.mode)


if __name__ == '__main__':
	main()


# endregion
