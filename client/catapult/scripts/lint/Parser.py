import re
import sys
import traceback
from collections import defaultdict
from enum import Enum
from xml.sax.saxutils import escape as xmlEscape

from colorPrint import Fore, Style, color_print, warning
from cppLexer import *  # pylint: disable=wildcard-import,unused-wildcard-import
from validation import Line

lex.lex()


class Mode(Enum):
    NORMAL = 1
    COLLECT_NS = 2
    COLLECT_TEMPLATE = 3
    COLLECT_CLASS = 4
    COLLECT_ENUM = 5
    INSIDE_CLASS_OR_ENUM = 6
    FIND_SEMICOLON = 7
    FIND_CLOSING_PAREN = 8
    FIND_CLOSING_BRACE = 9
    OPERATOR = 10


class NextTokenBehavior(Enum):
    SKIP = 1
    PICK = 2


PRINT_INFO = 0
PRINT_DEBUG = 0
PRINT_TRACE = 0

ANON_NS_FAKENAME = '<anon>'
TEXT_OUTPUT = False
DEST_DIR = '.'


def info(*args):
    if 1 == PRINT_INFO:
        color_print(Fore.GREEN, *args)


def debug(*args):
    if 1 == PRINT_DEBUG:
        color_print(Fore.GREEN, *args)


def trace_print(*args):
    if 1 == PRINT_TRACE:
        print(*args)


def has(token_name, stack):
    for token in stack:
        if token.type == token_name:
            return True
    return False


class TemplateError:  # pylint: disable=too-few-public-methods
    def __init__(self, name, line):
        self.name = name
        self.line = line


# pylint: disable=too-many-instance-attributes
class NamespaceInfo:  # pylint: disable=too-few-public-methods
    def __init__(self, current):
        self.properties = defaultdict(bool)
        self.current = current
        self.name = None

    def __getattr__(self, key):
        return self.properties[key]

    def __setattr__(self, key, value):
        if key in ('properties', 'current', 'name'):
            object.__setattr__(self, key, value)
        else:
            self.properties[key] = value

    # pylint: disable=attribute-defined-outside-init
    def __ior__(self, other):
        self.had_forward = self.had_forward or other.had_forward
        self.had_func_or_var = self.had_func_or_var or other.had_func_or_var
        self.had_class = self.had_class or other.had_class
        self.had_enum = self.had_enum or other.had_enum
        self.had_using = self.had_using or other.had_using
        self.had_test = self.had_test or other.had_test
        self.had_constant = self.had_constant or other.had_constant
        self.had_include = self.had_include or other.had_include
        # this is a hack for some catapult-specific macros usage
        self.had_define_macro = self.had_define_macro or other.had_define_macro
        return self

    def empty(self):
        return not (self.had_forward or
                    self.had_class or
                    self.had_enum or
                    self.had_func_or_var or
                    self.had_using or
                    self.had_test or
                    self.had_constant or
                    self.had_include or
                    self.had_define_macro)

    def __eq__(self, other):
        return self.name == other.name

    def __hash__(self):
        return hash(self.name)

    def __repr__(self):
        return '{} (fwd:{}, class:{}, enum:{}, funcOrVar:{}, using:{}, const:{} inc:{}, def:{})'.format(
            self.name or 'None',
            self.had_forward,
            self.had_class,
            self.had_enum,
            self.had_func_or_var,
            self.had_using,
            self.had_constant,
            self.had_include,
            self.had_define_macro)


# pylint: disable=too-many-instance-attributes
class NamespacesParser:
    def __init__(self, error_reporter, path):
        self.error_reporter = error_reporter
        self.path = path
        self.line_number = 0
        self.mode = Mode.NORMAL
        self.cur_ns_part = ''
        self.namespace_stack = []
        self.template_bracket_level = 0
        self.template_had_class = False
        self.template_content = []
        self.was_template_instantiation = False
        self.had_parens = False
        self.current_brace_level = 0
        self.current_colon_brace_level = 0
        self.name_stack = []
        self.current_paren_level = 0
        self.closing_brace_callback = None
        self.namespaces = set([])
        self.template_errors = []
        self.inside_template_callback = None

        self.tok = None
        info(self.path)
        self.parse_file(open(self.path, 'r'))

    def _quit_if_no_namestack(self, token):
        if not self.name_stack:
            self.quit(token)

    def parse_normal(self, tok):
        if tok.type == 'CLOSE_BRACE':
            self.add_namespace()
            return

        if tok.type in ('COLON', 'ASTERISK', 'COMMA', 'AMPERSTAND', 'OPEN_SQUARE_BRACKET', 'CLOSE_SQUARE_BRACKET'):
            self.name_stack.append(tok)
            return

        dispatch = {
            'NAME': self._parse_normal_name,
            'STRING_LITERAL': self._parse_normal_string_literal,
            'OPEN_BRACKET': self._parse_normal_open_bracket,
            'PRECOMP_MACRO': self._parse_normal_precomp_macro,
            'OPEN_PAREN': self._parse_normal_open_paren,
            'OPEN_BRACE': self._parse_normal_brace,
            'SEMI_COLON': self._parse_normal_semi_colon,
            'EQUALS': self._parse_normal_equals
        }

        if tok.type in dispatch:
            dispatch[tok.type](tok)
        else:
            self.quit(tok)

    def _parse_normal_name(self, tok):
        if tok.value == 'namespace':
            self.mode = Mode.COLLECT_NS
            self.cur_ns_part = ''
            debug('Collect NS')
        elif tok.value == 'template':
            self.inside_template_callback = None
            self.template_content = []
            self.collect_template(tok)
            self.mode = Mode.COLLECT_TEMPLATE
            debug('Collect Template')
        elif tok.value == 'class' or tok.value == 'struct':
            self.mode = Mode.COLLECT_CLASS
            self.name_stack.append(tok)
            debug('Collect ' + tok.value)
        elif tok.value == 'enum':
            self.mode = Mode.COLLECT_ENUM
            self.name_stack.append(tok)
            debug('Collect Enum')
        elif tok.value == 'using':
            self.mode = Mode.FIND_SEMICOLON
            self.name_stack.append(tok)
            debug('Collect Using')
        else:
            self.name_stack.append(tok)
            if tok.value == 'operator':
                self.mode = Mode.OPERATOR

    def _parse_normal_string_literal(self, tok):
        # Ignore extern "C"
        if not tok.value == '"C"':
            self.quit(tok)

    def _parse_normal_open_bracket(self, tok):
        self.inside_template_callback = lambda tok: self.name_stack.append(tok)  # pylint: disable=unnecessary-lambda
        self.template_content = []
        self.collect_template(tok)
        self.mode = Mode.COLLECT_TEMPLATE

    def _parse_normal_precomp_macro(self, tok):
        if self.namespace_stack and tok.value.startswith('#include'):
            self.namespace_stack[-1].had_include = True
            info('HAD INCLUDE')

        if not self.namespace_stack and re.match(r'#define [A-Z_]*TEST_CLASS ', tok.value):
            error_msg = '`#define TEST_CLASS` outside of any namespace'
            self.error_reporter('preprocessorOther', Line(self.path, '', tok.lineno, error_msg))

    def _parse_normal_open_paren(self, tok):
        debug('open paren')
        self._quit_if_no_namestack(tok)

        self.find_close_paren(tok)
        self.mode = Mode.FIND_CLOSING_PAREN

    def _parse_normal_brace(self, tok):
        self._quit_if_no_namestack(tok)

        self.find_close_brace(tok)
        trace_print('level brace open', self.current_brace_level, 'parens', self.had_parens)
        self.mode = Mode.FIND_CLOSING_BRACE
        if self.had_parens or self.name_stack[0].value == 'extern':
            self.closing_brace_callback = self.clear_name_stack
        else:
            self.closing_brace_callback = self.clear_name_stack_and_find_semicolon

    def _parse_normal_semi_colon(self, tok):
        self._quit_if_no_namestack(tok)

        if has('OPEN_PAREN', self.name_stack) and has('CLOSE_PAREN', self.name_stack):
            if self.namespace_stack:
                self.namespace_stack[-1].had_func_or_var = True
                info('HAD FUNC or VAR')

        self.clear_name_stack()

    def _parse_normal_equals(self, tok):
        self._quit_if_no_namestack(tok)
        self.name_stack.append(tok)
        self.mode = Mode.FIND_SEMICOLON

    def save_token_or_bye(self, previous_token, token, token_name):
        del previous_token
        if token.type == token_name:
            self.name_stack.append(token)
        else:
            self.quit(token)

    def _operator_equals(self, next_token):
        if next_token.type == 'EQUALS':
            self.name_stack.append(next_token)
            return NextTokenBehavior.PICK

        # operator=
        self.tok = next_token
        return NextTokenBehavior.SKIP

    def _operator_plus(self, next_token):
        # operator+= operator++
        if next_token.type == 'EQUALS' or next_token.type == 'PLUS':
            self.name_stack.append(next_token)
            return NextTokenBehavior.PICK

        # operator+
        self.tok = next_token
        return NextTokenBehavior.SKIP

    def _operator_minus(self, next_token):
        # operator-= operator-- operator->
        if next_token.type == 'EQUALS' or next_token.type == 'MINUS' or next_token.type == 'CLOSE_BRACKET':
            self.name_stack.append(next_token)
            return NextTokenBehavior.PICK

        # operator-
        self.tok = next_token
        return NextTokenBehavior.SKIP

    def _operator_less_than(self, next_token):
        # operator <<, <=
        if next_token.type == 'OPEN_BRACKET' or next_token.type == 'EQUALS':
            self.name_stack.append(next_token)
            return NextTokenBehavior.PICK

        # operator<
        self.tok = next_token
        return NextTokenBehavior.SKIP

    def _operator_greater_than(self, next_token):
        # operator >=
        if next_token.type == 'EQUALS':
            self.name_stack.append(next_token)
            return NextTokenBehavior.PICK

        # operator>
        self.tok = next_token
        return NextTokenBehavior.SKIP

    # pylint: disable=too-many-branches
    def collect_operator(self):
        tok = lex.token()
        self.name_stack.append(tok)
        tok2 = lex.token()
        behavior = NextTokenBehavior.PICK
        # operator()
        if tok.type == 'OPEN_PAREN':
            self.save_token_or_bye(tok, tok2, 'CLOSE_PAREN')
        # operator[]
        elif tok.type == 'OPEN_SQUARE_BRACKET':
            self.save_token_or_bye(tok, tok2, 'CLOSE_SQUARE_BRACKET')
        # operator==
        elif tok.type == 'EQUALS':
            behavior = self._operator_equals(tok2)
        elif tok.type == 'PLUS':
            behavior = self._operator_plus(tok2)
        elif tok.type == 'MINUS':
            behavior = self._operator_minus(tok2)
        # operator!=
        elif tok.type == 'EXCLAMATION':
            self.save_token_or_bye(tok, tok2, 'EQUALS')
        # operator|
        elif tok.type == 'PIPE':
            self.tok = tok2
            return
        # operator bool
        elif tok.type == 'NAME' and tok.value == 'bool':
            self.tok = tok2
            return
        # operator*
        elif tok.type == 'ASTERISK':
            if tok2.type != 'OPEN_PAREN':
                self.quit(tok2)
            self.tok = tok2
            return
        elif tok.type == 'OPEN_BRACKET':
            behavior = self._operator_less_than(tok2)
        elif tok.type == 'CLOSE_BRACKET':
            behavior = self._operator_greater_than(tok2)
        else:
            self.quit(tok2)

        if behavior == NextTokenBehavior.PICK:
            self.tok = lex.token()

    def add_namespace(self):
        name = '::'.join(map(lambda c: c.current, self.namespace_stack))
        if ANON_NS_FAKENAME in name:
            if self.path.endswith('.h'):
                self.error_reporter('anonNamespace', Line(self.path, name, self.tok.lineno))

        current = self.namespace_stack.pop()
        current.name = name
        dummy = NamespaceInfo('')
        dummy.name = name
        if dummy not in self.namespaces:
            self.namespaces.add(current)
            info('adding namespace ', current)
        else:
            for i in self.namespaces:
                if i != dummy:
                    continue
                i |= current
            info('merging namespace ', current)

    def add_template_error(self, line):
        name = '::'.join(map(lambda c: c.current, self.namespace_stack))
        dummy = TemplateError(name, line)
        self.template_errors.append(dummy)

    def clear_name_stack(self):
        self.name_stack = []
        self.closing_brace_callback = None

    def clear_name_stack_and_find_semicolon(self):  # pylint: disable=invalid-name
        self.tok = lex.token()
        self.name_stack.append(self.tok)
        if self.tok.type == 'SEMI_COLON':
            self.switch_to_normal()
            self.check_properties()
        else:
            self.quit(self.tok)

        self.name_stack = []
        self.closing_brace_callback = None

    def parse_file(self, input_stream):
        lex.input(input_stream.read())
        while True:
            if self.mode == Mode.OPERATOR:
                self.collect_operator()
                self.mode = Mode.NORMAL
            else:
                self.tok = lex.token()
                trace_print(self.mode, self.tok)
            if not self.tok:
                break
            dispatch = {
                Mode.NORMAL: self.parse_normal,
                Mode.COLLECT_NS: self.collect_ns,
                Mode.COLLECT_TEMPLATE: self.collect_template,
                Mode.COLLECT_CLASS: self.collect_class,
                Mode.COLLECT_ENUM: self.collect_enum,
                Mode.INSIDE_CLASS_OR_ENUM: self.find_class_end,
                Mode.FIND_SEMICOLON: self.find_semi_colon,
                Mode.FIND_CLOSING_PAREN: self.find_close_paren,
                Mode.FIND_CLOSING_BRACE: self.find_close_brace
            }
            dispatch[self.mode](self.tok)

    def quit(self, tok):
        if not TEXT_OUTPUT:
            with open(DEST_DIR + '/tests.fatalerror.xml', 'w') as output_stream:
                output_stream.write('<?xml version="1.0" encoding="UTF-8"?>\n')
                output_stream.write('<testsuites tests="0" failures="0" disabled="0" errors="1" time="0" name="AllTests">\n')
                output_stream.write('  <testsuite name="Parser" tests="0" failures="0" disabled="0" errors="1" time="0">\n')
                msg = 'Error while parsing the file: {}\n'.format(self.path)
                for _ in range(20):
                    msg += repr(tok.type) + ' : ' + repr(tok.value) + '\n'
                    tok = lex.token()
                    if not tok:
                        break

                output_stream.write('  <testcase status="run" time="0" classname="parser" name="ERROR">\n')
                output_stream.write('    <error message="{}" type=""><![CDATA[{}]]></error>\n'.format(xmlEscape(msg), msg))
                output_stream.write('  </testcase>\n')
                output_stream.write('  </testsuite>\n')
                output_stream.write('</testsuites>\n')

        else:
            print(Fore.RED + Style.BRIGHT + 'Quitting ' + self.path + Style.RESET_ALL)
            for line in traceback.format_stack():
                print(line.strip())
            for _ in range(20):
                print(repr(tok.type), repr(tok.value))
                tok = lex.token()
                if not tok:
                    break
        sys.exit(1)

    def switch_to_normal(self):
        self.mode = Mode.NORMAL
        self.had_parens = False

    def collect_ns(self, tok):
        if tok.type == 'NAME':
            self.cur_ns_part = tok.value
        elif tok.type == 'OPEN_BRACE':
            if not self.cur_ns_part:
                self.cur_ns_part = ANON_NS_FAKENAME
            namespace = NamespaceInfo(self.cur_ns_part)
            self.namespace_stack.append(namespace)
            self.switch_to_normal()
        elif tok.type == 'EQUALS':
            self.mode = Mode.FIND_SEMICOLON
            debug('got namespace rename "{}"'.format(self.cur_ns_part))

    def collect_template(self, tok):
        self.template_content.append(tok)
        if tok.type == 'OPEN_BRACKET':
            self.template_bracket_level += 1
        elif tok.type == 'CLOSE_BRACKET':
            self.template_bracket_level -= 1
            if self.template_bracket_level == 0:
                if self.template_had_class:
                    line = ' '.join(map(lambda e: e.value, self.template_content))
                    self.add_template_error(line)
                    self.template_had_class = False

                if self.was_template_instantiation:
                    self.mode = Mode.FIND_SEMICOLON
                    self.was_template_instantiation = False
                else:
                    self.switch_to_normal()

        # template instantiation
        elif tok.type == 'NAME' and (tok.value == 'class' or tok.value == 'struct'):
            if self.template_bracket_level > 0:
                self.template_had_class = True
            if self.inside_template_callback:
                self.inside_template_callback(tok)
            self.was_template_instantiation = True
            tok = lex.token()

        if self.inside_template_callback:
            self.inside_template_callback(tok)

    def collect_class(self, tok):
        if tok.type == 'OPEN_BRACE':
            self.current_brace_level += 1
            self.mode = Mode.INSIDE_CLASS_OR_ENUM
            if self.namespace_stack:
                self.namespace_stack[-1].had_class = True
                info('HAD CLASS')
            else:
                warning('WARNING: Class at ROOT namespace found: ', self.path)

        elif tok.type == 'SEMI_COLON':
            self.switch_to_normal()

            # do not consider forward declarations in global scope
            # (they do not add anything to namespace)
            if not self.namespace_stack:
                # no need to clear_name_stack() cause it is empty
                return

            self.namespace_stack[-1].had_forward = True

            name = '::'.join(map(lambda c: c.current, self.namespace_stack))
            obj_name = ' '.join(map(lambda c: c.value, self.name_stack))
            info('HAD FORWARD', name, obj_name)
            self.clear_name_stack()
        else:
            self.name_stack.append(tok)

    def collect_enum(self, tok):
        if tok.type == 'OPEN_BRACE':
            self.current_brace_level += 1
            self.mode = Mode.INSIDE_CLASS_OR_ENUM
            if self.namespace_stack:
                self.namespace_stack[-1].had_enum = True
                info('HAD ENUM')
            else:
                warning('WARNING: Enum at ROOT namespace found: ', self.path)

        elif tok.type == 'SEMI_COLON':
            self.switch_to_normal()
            self.namespace_stack[-1].had_forward = True

            name = '::'.join(map(lambda c: c.current, self.namespace_stack))
            obj_name = ' '.join(map(lambda c: c.value, self.name_stack))
            info('HAD FORWARD', name, obj_name)
            self.clear_name_stack()

        else:
            self.name_stack.append(tok)

    def find_class_end(self, tok):
        if self.current_brace_level == 0:
            if tok.type == 'SEMI_COLON':
                self.switch_to_normal()
                self.clear_name_stack()
            else:
                self.quit(tok)
        if tok.type == 'OPEN_BRACE':
            self.current_brace_level += 1
        if tok.type == 'CLOSE_BRACE':
            self.current_brace_level -= 1

    def find_close_brace(self, tok):
        if tok.type == 'OPEN_BRACE':
            if 0 == self.current_brace_level:
                self.name_stack.append(tok)
            self.current_brace_level += 1
            trace_print('level brace open', self.current_brace_level)
        elif tok.type == 'CLOSE_BRACE':
            self.current_brace_level -= 1
            trace_print('level brace close', self.current_brace_level)
            if self.current_brace_level == 0:
                self.name_stack.append(tok)
                self.switch_to_normal()
                self.check_properties()
                self.closing_brace_callback()

    def _check_func_or_var(self):
        if has('OPEN_BRACE', self.name_stack) and has('CLOSE_BRACE', self.name_stack):
            if self.namespace_stack:
                self.namespace_stack[-1].had_func_or_var = True
                info('HAD FUNC or VAR')
            else:
                function_name = ' '.join(map(lambda e: e.value, self.name_stack))
                info('WARNING: Probably function at ROOT namespace found: ', self.path, function_name)
        if has('OPEN_SQUARE_BRACKET', self.name_stack) and has('CLOSE_SQUARE_BRACKET', self.name_stack):
            if self.namespace_stack:
                self.namespace_stack[-1].had_func_or_var = True
                info('HAD FUNC or VAR')
            else:
                warning('WARNING: Unknown case: ', self.path, ' '.join(map(lambda e: e.value, self.name_stack)))

    def check_properties(self):
        if self.name_stack[0].value == 'using':
            if self.namespace_stack:
                self.namespace_stack[-1].had_using = True
                info('HAD USING')
        elif self.name_stack[0].value == 'TEST':
            if self.namespace_stack:
                self.namespace_stack[-1].had_test = True
                info('HAD TEST')
        elif self.name_stack[0].value == 'extern':
            # Ignore
            pass
        else:
            self._check_func_or_var()

            # if constexpr and has assignment
            if self.name_stack[0].value == 'constexpr' and has('EQUALS', self.name_stack):
                self.namespace_stack[-1].had_constant = True
                info('HAD Constant')

    def find_semi_colon(self, tok):
        if self.current_colon_brace_level == 0:
            self.name_stack.append(tok)
            if tok.type == 'SEMI_COLON':
                self.switch_to_normal()
                self.check_properties()
                self.clear_name_stack()

        if tok.type == 'OPEN_BRACE':
            self.current_colon_brace_level += 1
            trace_print('level colon brace open', self.current_colon_brace_level)
        if tok.type == 'CLOSE_BRACE':
            self.current_colon_brace_level -= 1
            trace_print('level colon brace clode', self.current_colon_brace_level)

    def find_close_paren(self, tok):
        self.name_stack.append(tok)
        if tok.type == 'OPEN_PAREN':
            self.current_paren_level += 1
            trace_print('level after open ', self.current_paren_level)
        if tok.type == 'CLOSE_PAREN':
            self.current_paren_level -= 1
            trace_print('level after close ', self.current_paren_level)
            if self.current_paren_level == 0:
                if self.namespace_stack:
                    if self.name_stack and self.name_stack[0].value.startswith('DEFINE_'):
                        self.namespace_stack[-1].had_define_macro = True
                        info('HAD DEFINE_* macro')
                self.switch_to_normal()
                self.had_parens = True
                debug('close paren {} namespace_stack:{}'.format(self.had_parens, len(self.namespace_stack)))
