import os
import re
from enum import Enum
from pathlib import Path


class ParserState(Enum):
	UNDEFINED = 0
	COLLECTING = 1


def parse_code(infile):
	"""Parses sdk/python/examples/docs/__main__.py, creates function name -> code mapping."""

	func_pattern = re.compile(r'(async )?(def|class) (?P<func_name>\w+)')
	parsed_code = {}

	# this is super _crude_, it only handles func starting at the beginning of line
	method_name = None
	method_lines = []
	parser_state = ParserState.UNDEFINED

	def add_method():
		nonlocal method_name
		nonlocal method_lines
		nonlocal parsed_code

		if not method_name:
			return

		# remove empty lines at the end
		while not method_lines[-1]:
			method_lines.pop()

		parsed_code[method_name] = method_lines
		method_lines = []
		method_name = None

	for line in infile:
		line = line.rstrip('\n')
		if re.match(r'^[^\t\n]', line):
			parser_state = ParserState.UNDEFINED
			add_method()

		if parser_state == ParserState.UNDEFINED:
			match = re.match(func_pattern, line)
			if match:
				# there is already a collected method, save it
				add_method()

				method_name = match.group('func_name')
				method_lines.append(line)
				parser_state = ParserState.COLLECTING

		elif parser_state == ParserState.COLLECTING:
			method_lines.append(line)

	return parsed_code


def insert_fenced_example(outfile, lines, indent):
	outfile.write(f'{indent}```python\n')
	for line in lines:
		outfile.write(f'{indent}{line}\n')
	outfile.write(f'{indent}```\n')


def process(parsed_code, infile, outfile):
	"""Parse big.md file, inserting !inline-s and !example-s."""
	processor_pattern = re.compile(r'^(?P<indent>\t+)?(?P<command>!inline|!example) (?P<name>\S*)( @(?P<directory>.*))?')

	for line in infile:
		match = re.match(processor_pattern, line)

		if match:
			command = match.group('command')
			name = match.group('name')
			if '!inline' == command:
				directory = match.group('directory')
				inlined_filename = (Path(directory) if directory else Path()) / 'docs' / f'{name}.md'
				with open(inlined_filename, encoding='utf8') as inlined_file:
					process(parsed_code, inlined_file, outfile)
			elif '!example' == command:
				indent = match.group('indent') or ''
				insert_fenced_example(outfile, parsed_code[name], indent)
		else:
			outfile.write(line)


def main():
	root_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
	os.chdir(root_dir)

	with open(f'sdk/python/examples/docs/__main__.py', 'r', encoding='utf8') as infile:
		parsed_code = parse_code(infile)

	with open('docs/big.md', 'r', encoding='utf8') as infile:
		with open('docs/index.md', 'w', encoding='utf8') as outfile:
			process(parsed_code, infile, outfile)


if '__main__' == __name__:
	main()
