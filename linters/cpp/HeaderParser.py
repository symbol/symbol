# pylint: disable=too-few-public-methods

import os
import re
import shutil
from enum import Enum

from validation import Line


class PpType(Enum):
	INCLUDE = 1
	DEFINE = 2
	UNDEF = 3
	IFDEF = 4
	ELSE = 5
	ENDIF = 6
	PRAGMA = 7
	ERROR = 8


class Preproc:
	def __init__(self, line, lineno, word):
		self.line = line
		self.lineno = lineno
		match_to_type = {
			'define': PpType.DEFINE,
			'undef': PpType.UNDEF,
			'extern': PpType.IFDEF,
			'if': PpType.IFDEF,
			'ifdef': PpType.IFDEF,
			'ifndef': PpType.IFDEF,
			'elif': PpType.ELSE,
			'else': PpType.ELSE,
			'endif': PpType.ENDIF,
			'pragma': PpType.PRAGMA,
			'error': PpType.ERROR
		}
		if word in match_to_type:
			self.type = match_to_type[word]
		else:
			raise RuntimeError('unknown preprocessor directive' + line)

	def __str__(self):
		return '{}'.format(self.line)


class Include:
	def __init__(self, line, lineno, include, rest):
		self.type = PpType.INCLUDE
		self.line = line
		self.lineno = lineno
		self.include = include
		self.rest = rest

	def __str__(self):
		return '#include {}{}'.format(self.include, self.rest)


class MultilineMacro(Enum):
	PPLINE = 1
	CONTINUATION = 2


class IndentFix:
	def __init__(self, mmtype, lineno, line):
		self.type = mmtype
		self.lineno = lineno
		self.line = line


def fix_tabs(line, count):
	if count == 0:
		return line

	# remove count tabs
	if count < 0:
		line = re.sub(r'\t', '', line, -count)
	else:
		line = '\t' * count + line
	return line


class HeaderParser:
	PATTERN_INCLUDE = re.compile(r'\s*#\s*include[ \t]*(["<][^">]*[">])(.*)')
	PATTERN_PREPROCESSOR = re.compile(r'\s*#\s*(\w*)')
	PATTERN_EXTERN = re.compile(r'\s*extern\s*')
	PATTERN_EMPTY_LINE = re.compile(r'^\s*$')

	def __init__(self, error_reporter, path, simple_validators, fix_indents_in_files=False):
		self.error_reporter = error_reporter
		self.path = path
		self.line_number = 0
		self.preprocessor = []
		#
		self.includes = []
		self.include_error = None
		self.simple_validators = simple_validators

		self.fixes = []
		# need to open as binary so that python can see '\r\n'
		with open(self.path, 'rb') as input_file:
			self.parse_file(input_file)

		if self.fixes:
			if fix_indents_in_files:
				with open(self.path, 'r', encoding='utf8') as input_file:
					with open(self.path + '.tmp', 'w', encoding='utf8') as output_file:
						self.fix_indents(input_file, output_file)

				os.remove(self.path)
				shutil.move(self.path + '.tmp', self.path)
			self.report_indents()

	def fix_indents(self, inf, outf):
		fix_no = 0
		line_number = 1
		first_continuation = False
		for line in inf:
			line = line.strip('\n')
			if fix_no < len(self.fixes) and line_number == self.fixes[fix_no].lineno:
				line = line.strip('\n')
				if MultilineMacro.PPLINE == self.fixes[fix_no].type:
					line = line.strip()
					first_continuation = True
				else:
					if first_continuation:
						tabs_match = re.match(r'^\t+', line)
						tabs_count = len(tabs_match.group(0)) if tabs_match else 0
						first_continuation = False
					line = fix_tabs(line, 1 - tabs_count)
				fix_no += 1
			line = line + '\n'
			line_number += 1
			outf.write(line.encode('utf-8'))

	def report_indents(self):
		first_continuation = False
		for fix in self.fixes:
			if MultilineMacro.PPLINE == fix.type:
				if re.match(r'\s+', fix.line):
					error_msg = 'preprocessor should be aligned to column 0'
					self.error_reporter('indentedPreprocessor', Line(self.path, fix.line.strip('\n\r'), fix.lineno, error_msg))
					first_continuation = True
			else:
				if first_continuation:
					tabs_match = re.match(r'^\t+', fix.line)
					tabs_count = len(tabs_match.group(0)) if tabs_match else 0
					if 1 != tabs_count:
						error_msg = 'first continuation must have single indent'
						self.error_reporter('indentedPreprocessor', Line(self.path, fix.line.strip('\n\r'), fix.lineno, error_msg))
					first_continuation = False

	def process_continuation(self, line):
		self.fixes.append(IndentFix(MultilineMacro.CONTINUATION, self.line_number, line))
		return line and '\\' == line[-1]

	def process_preprocessor(self, line):
		multiline = False
		if '\\' == line[-1]:
			multiline = True
		# this is here to skip pragma from preprocessor indent check
		if line != '#pragma once':
			self.fixes.append(IndentFix(MultilineMacro.PPLINE, self.line_number, line))
		return multiline

	def parse_file(self, input_stream):
		self.line_number = 1
		pprev = None
		prev = None
		temp = None
		is_empty_line = False
		prev_empty_line = False
		multiline = False

		for validator in self.simple_validators:
			validator.reset(self.path, self.error_reporter)

		for raw_line in input_stream:
			line = raw_line.decode('utf8')
			line = line.strip('\n')

			pprev = prev
			prev = temp
			temp = re.sub('\t', '    ', line)

			for validator in self.simple_validators:
				validator.check(self.line_number, line)

			prev_empty_line = is_empty_line
			is_empty_line = bool(self.PATTERN_EMPTY_LINE.match(line))
			if is_empty_line and prev_empty_line:
				self.error_reporter('consecutiveEmpty', Line(self.path, pprev + prev + temp, self.line_number))

			if multiline:
				multiline = self.process_continuation(line)
			else:
				if self.PATTERN_INCLUDE.match(line):
					self.parse_include(line)
					self.fixes.append(IndentFix(MultilineMacro.PPLINE, self.line_number, line))
				elif self.PATTERN_PREPROCESSOR.match(line):
					self.parse_preprocessor(line)
					multiline = self.process_preprocessor(line)
				elif self.PATTERN_EXTERN.match(line):
					self.parse_extern(line)
			self.line_number += 1

		if prev and not prev.strip():
			self.error_reporter('emptyNearEnd', Line(self.path, pprev + prev + temp, self.line_number))

		for validator in self.simple_validators:
			validator.finalize()

	def parse_include(self, line):
		res = self.PATTERN_INCLUDE.match(line)
		self.preprocessor.append(Include(line, self.line_number, res.group(1), res.group(2)))
		self.includes.append(res.group(1))

	def parse_preprocessor(self, line):
		res = self.PATTERN_PREPROCESSOR.match(line)
		self.preprocessor.append(Preproc(line, self.line_number, res.group(1)))

	def parse_extern(self, line):
		self.preprocessor.append(Preproc(line, self.line_number, 'extern'))
