import re
from enum import Enum

from colorPrint import Fore, color_print
from cppLexer import *  # pylint: disable=wildcard-import,unused-wildcard-import
from exclusions import FILTER_NAMESPACES, SKIP_FORWARDS
from SimpleValidator import Line, SimpleValidator

PRINT_INFO = False


def info(*args):
	if PRINT_INFO:
		color_print(Fore.GREEN, *args)


class SimpleNsTokenizer:
	def __init__(self):
		self.line = None
		self.lex = lex
		self.lex.lex()

	def feed(self, line):
		lex.input(line)
		self.line = line

	def __iter__(self):
		return self

	def find(self):
		tok = self.lex.token()
		if not tok:
			raise StopIteration()
		if tok.type in [
			'NAME', 'OPEN_BRACE', 'CLOSE_BRACE', 'SEMI_COLON', 'OPEN_BRACKET', 'CLOSE_BRACKET',
			'COLON', 'EQUALS', 'OPEN_PAREN', 'CLOSE_PAREN', 'AMPERSTAND', 'COMMA', 'MINUS',
			'ASTERISK', 'EXCLAMATION', 'OPEN_SQUARE_BRACKET', 'CLOSE_SQUARE_BRACKET', 'NUMBER',
			'PRECOMP_MACRO', 'BACKSLASH'
		]:
			return (tok.lexpos, tok.value)
		raise RuntimeError('unknown token {}'.format(tok))

	def __next__(self):
		ret = self.find()
		return (ret[0], ret[1])


class Mode(Enum):
	NORMAL = 1
	COLLECT_NS_NAME = 2
	COLLECT_OPENING_BRACE = 3
	COLLECT_TYPE_NAME = 4
	COLLECT_SEMICOLON = 5
	SKIP_UNTIL_EOL = 6
	COLLECT_NAMESPACE_KEYWORD = 7
	COLLECT_ENUM = 8


class NamespaceType(Enum):
	NORMAL = 1
	INLINE = 2


def create_dict():
	return {'namespaces': {}, 'forwards': {}}


# pylint: disable=too-many-instance-attributes
class ForwardsValidator(SimpleValidator):
	SUITE_NAME = 'ForwardDeclaration'
	NAME = 'forward'

	def __init__(self):
		super().__init__()
		self.pattern_namespace = re.compile(r'\s*namespace')

	# pylint: disable=attribute-defined-outside-init
	def reset(self, path, error_reporter):
		super().reset(path, error_reporter)
		self.match_line_number = 0
		self.parsing_done = False
		self.mode = Mode.NORMAL
		self.namespace_type = None
		self.tokenizer = SimpleNsTokenizer()
		self.tok = None
		self.namespace_stack = []
		self.declarations = create_dict()
		self.type_type = None
		self.last_name = None
		self.pre_type_lines = []
		self.had_forwards = False
		self.last_forward_line_number = 0
		self.last_line_number = None
		self.last_namespace_line_number = None
		self.last_namespace_next_line_length = 0
		self.previous_block_line_number = None
		self.collected_lines = []

	def check(self, line_number, line):
		if 0 == self.match_line_number:
			result = self.pattern_namespace.match(line)
			if not result:
				return
			self.match_line_number = line_number
		self._parse(line_number, line)

	def _parse(self, line_number, line):
		if self.parsing_done:
			return

		self.last_line_number = line_number
		self.collected_lines.append(line)

		if self.last_namespace_line_number and self.last_namespace_line_number + 1 == line_number:
			self.last_namespace_next_line_length = len(line)

		self.tokenizer.feed(line + '\n')
		self.line = line
		dispatch = {
			Mode.NORMAL: self._keyword_check,
			Mode.COLLECT_NAMESPACE_KEYWORD: self._collect_namespace_keyword,
			Mode.COLLECT_NS_NAME: self._collect_ns_name,
			Mode.COLLECT_OPENING_BRACE: self._collect_opening_brace,
			Mode.COLLECT_TYPE_NAME: self._collect_type_name,
			Mode.COLLECT_ENUM: self._collect_enum,
			Mode.COLLECT_SEMICOLON: self._collect_semi_colon,
			Mode.SKIP_UNTIL_EOL: self._skip_till_eol
		}
		for tok in self.tokenizer:
			if self.parsing_done:
				return

			self.tok_pos = tok[0]
			self.tok = tok[1]
			dispatch[self.mode]()

		if self.mode == Mode.SKIP_UNTIL_EOL:
			self.mode = Mode.NORMAL

	def _keyword_check(self):
		if self.tok == 'namespace':
			self.mode = Mode.COLLECT_NS_NAME
			self.namespace_type = NamespaceType.NORMAL
		elif self.tok == 'inline':
			self.mode = Mode.COLLECT_NAMESPACE_KEYWORD
		elif self.tok in ('class', 'struct'):
			self.mode = Mode.COLLECT_TYPE_NAME
			self.type_type = self.tok
		elif self.tok == 'enum':
			self.mode = Mode.COLLECT_ENUM
			self.type_type = self.tok
		elif self.tok == 'template':
			self.mode = Mode.SKIP_UNTIL_EOL
			self.pre_type_lines.append(self.line[self.tok_pos:])
		elif self.tok == 'extern':
			# we assume it's a single line extern and simply ignore the content
			self.mode = Mode.SKIP_UNTIL_EOL
		elif self.tok.startswith('#'):
			self.mode = Mode.SKIP_UNTIL_EOL
		elif self.tok.startswith('//'):
			self.mode = Mode.SKIP_UNTIL_EOL
			self.pre_type_lines.append(self.line[self.tok_pos:])
		elif self.tok == '}':
			self.namespace_stack.pop()
		elif self.tok == '\n':
			pass
		else:
			info('final token', self.tok)
			self.parsing_done = True

	def _get_path(self):
		current = self.declarations
		for namespace_name in self.namespace_stack:
			current = current['namespaces'][namespace_name]
		return current

	def _collect_namespace_keyword(self):
		if 'namespace' == self.tok:
			self.mode = Mode.COLLECT_NS_NAME
			self.namespace_type = NamespaceType.INLINE
		else:
			info('final token', self.tok)
			self.parsing_done = True

	def _collect_ns_name(self):
		if not self.namespace_stack:
			self.previous_block_line_number = self.last_line_number

		if '{' == self.tok:
			info('final token, anon namespace')
			self.parsing_done = True
			return

		current = self._get_path()
		if self.tok not in current['namespaces']:
			current['namespaces'][self.tok] = create_dict()
			current['namespaces'][self.tok]['type'] = self.namespace_type

		self.namespace_stack.append(self.tok)
		self.mode = Mode.COLLECT_OPENING_BRACE

		fq_namespace = '::'.join(self.namespace_stack)
		for filtered in FILTER_NAMESPACES:
			if re.match(filtered, fq_namespace):
				break
		else:
			self.last_namespace_line_number = self.last_line_number

	def _add_forward(self, type_type, type_name):
		if not (self.type_type and self.last_name):
			return

		current = self._get_path()
		if type_name in current['forwards']:
			self.error_reporter(self.NAME, Line(self.path, type_name, self.last_line_number))

		current['forwards'][type_name] = {'type': type_type, 'pre': self.pre_type_lines}

		self.had_forwards = True
		self.last_forward_line_number = self.last_line_number
		self.type_type = None
		self.last_name = None
		self.pre_type_lines = []

	def _collect_opening_brace(self):
		self.mode = Mode.NORMAL
		if not '{' == self.tok:
			info('invalid token >>{}<<, expected: {}'.format(self.tok, '{'))
			self.parsing_done = True

	def _collect_semi_colon(self):
		self.mode = Mode.NORMAL
		if ';' == self.tok:
			self._add_forward(self.type_type, self.last_name)
		else:
			info('invalid token >>{}<<, expected: ;'.format(self.tok))
			self.parsing_done = True

	def _collect_type_name(self):
		self.last_name = self.tok
		self.mode = Mode.COLLECT_SEMICOLON

	def _collect_enum(self):
		if 'class' == self.tok:
			self.type_type = 'enum class'
			return

		if not self.last_name:
			self.last_name = self.tok
			return

		if ':' in self.last_name and self.last_name.endswith(' :'):
			self.last_name += ' {}'.format(self.tok)
			return

		if ':' == self.tok:
			self.last_name += ' {}'.format(self.tok)
			return

		if ';' == self.tok:
			self._collect_semi_colon()
		else:
			info('invalid token >>{}<<, expected: ; in >>{} {}<<'.format(self.tok, self.type_type, self.last_name))
			self.parsing_done = True

	def _skip_till_eol(self):
		if self.tok == '\n':
			self.mode = Mode.NORMAL

	@staticmethod
	def _format_fwd(class_name, class_spec):
		formatted = '{} {};'.format(class_spec['type'], class_name)
		if not class_spec['pre']:
			return [formatted]

		temp = class_spec['pre'][:]
		temp.append(formatted)
		return temp

	@staticmethod
	def indent_not_empty(line):
		return '\t' + line if line else line

	@staticmethod
	def _format(declarations, level=1):
		result = []
		if declarations['namespaces']:
			# deliberately start from 1, to simplify condition at the bottom of the loop
			for namespace_name, sub_declarations in sorted(declarations['namespaces'].items()):
				lines = ForwardsValidator._format(sub_declarations, level + 1)
				ns_type = declarations['namespaces'][namespace_name]['type']
				content = 'inline ' if NamespaceType.INLINE == ns_type else ''

				if not lines:
					continue

				if 1 == level and result:
					result.append('')

				if len(lines) == 1:
					content += 'namespace %s { %s }' % (namespace_name, lines[0])
					result.append(content)

				else:
					content += 'namespace %s {' % namespace_name
					result.append(content)
					indent_lines = map(ForwardsValidator.indent_not_empty, lines)
					result.extend(indent_lines)
					result.append('}')

		if declarations['forwards']:
			forwards = []
			for class_name, class_spec in sorted(declarations['forwards'].items(), key=lambda x: x[0].lower()):
				current_forward = ForwardsValidator._format_fwd(class_name, class_spec)

				# if forward has anything before it (comment or template<>)
				# prepend additional empty line
				if forwards and len(current_forward) > 1:
					forwards.append('')
				forwards.extend(current_forward)
			result.extend(forwards)

		return result

	def finalize(self):
		report_forwards_error = True
		for pattern in SKIP_FORWARDS:
			if re.match(pattern, self.path):
				report_forwards_error = False
				break

		if self.last_namespace_next_line_length and self.last_namespace_line_number > self.last_forward_line_number:
			self.error_reporter(self.NAME, Line(self.path, '', self.last_namespace_line_number + 1, 'missingEmptyLine'))

		if report_forwards_error and self.had_forwards:
			expected_lines = self._format(self.declarations)
			expected = '\n'.join(expected_lines)
			current = '\n'.join(self.collected_lines[:len(expected_lines)])
			if current != expected:
				self.error_reporter(
					self.NAME,
					Line(self.path, current, (self.match_line_number, self.previous_block_line_number - 1), expected))

	@staticmethod
	def format_error(err):
		if isinstance(err.lineno, tuple):
			name = err.path
			error_fmt = '{}:{} - {} forward declarations mismatch, has:\n{}\nshould be:\n{}'
			return error_fmt.format(name, err.lineno[0], err.lineno[1], err.line, err.kind)

		if err.kind == 'missingEmptyLine':
			name = err.path
			error_fmt = '{}:{} - missing empty line after namespace'
			return error_fmt.format(name, err.lineno, err.line)

		name = err.path
		error_fmt = '{}:{} - redefinition of a type forward declaration: {}'
		return error_fmt.format(name, err.lineno, err.line)
