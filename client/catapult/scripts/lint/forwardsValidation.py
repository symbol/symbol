import re
from enum import Enum

from colorPrint import colorPrint, Fore
from cppLexer import *  # pylint: disable=wildcard-import,unused-wildcard-import
from SimpleValidator import SimpleValidator, Line
from exclusions import SKIP_FORWARDS, FILTER_NAMESPACES

PRINT_INFO = False


def info(*args):
    if PRINT_INFO:
        colorPrint(Fore.GREEN, *args)


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
        if tok.type in ['NAME', 'OPEN_BRACE', 'CLOSE_BRACE', 'SEMI_COLON', 'OPEN_BRACKET', 'CLOSE_BRACKET',
                        'COLON', 'EQUALS', 'OPEN_PAREN', 'CLOSE_PAREN', 'AMPERSTAND', 'COMMA', 'MINUS',
                        'ASTERISK', 'EXCLAMATION', 'OPEN_SQUARE_BRACKET', 'CLOSE_SQUARE_BRACKET', 'NUMBER',
                        'PRECOMP_MACRO', 'BACKSLASH']:
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


def createDict():
    return {'namespaces': {}, 'forwards': {}}


# pylint: disable=too-many-instance-attributes
class ForwardsValidator(SimpleValidator):
    SUITE_NAME = 'ForwardDeclaration'
    NAME = 'forward'

    def __init__(self):
        super().__init__()
        self.patternNamespace = re.compile(r'\s*namespace')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.matchLineNumber = 0
        self.parsingDone = False
        self.mode = Mode.NORMAL
        self.namespaceType = None
        self.tokenizer = SimpleNsTokenizer()
        self.tok = None
        self.namespaceStack = []
        self.declarations = createDict()
        self.typeType = None
        self.lastName = None
        self.preTypeLines = []
        self.hadForwards = False
        self.lastForwardLineNumber = 0
        self.lastLineNumber = None
        self.lastNamespaceLineNumber = None
        self.lastNamespaceNextLineLength = 0
        self.previousBlockLineNumber = None
        self.collectedLines = []

    def check(self, lineNumber, line):
        if 0 == self.matchLineNumber:
            result = self.patternNamespace.match(line)
            if not result:
                return
            self.matchLineNumber = lineNumber
        self._parse(lineNumber, line)

    def _parse(self, lineNumber, line):
        if self.parsingDone:
            return

        self.lastLineNumber = lineNumber
        self.collectedLines.append(line)

        if self.lastNamespaceLineNumber and self.lastNamespaceLineNumber + 1 == lineNumber:
            self.lastNamespaceNextLineLength = len(line)

        self.tokenizer.feed(line + '\n')
        self.line = line
        dispatch = {
            Mode.NORMAL: self._keywordCheck,
            Mode.COLLECT_NAMESPACE_KEYWORD: self._collectNamespaceKeyword,
            Mode.COLLECT_NS_NAME: self._collectNsName,
            Mode.COLLECT_OPENING_BRACE: self._collectOpeningBrace,
            Mode.COLLECT_TYPE_NAME: self._collectTypeName,
            Mode.COLLECT_ENUM: self._collectEnum,
            Mode.COLLECT_SEMICOLON: self._collectSemiColon,
            Mode.SKIP_UNTIL_EOL: self._skipTillEol
        }
        for tok in self.tokenizer:
            if self.parsingDone:
                return

            self.tokPos = tok[0]
            self.tok = tok[1]
            dispatch[self.mode]()

        if self.mode == Mode.SKIP_UNTIL_EOL:
            self.mode = Mode.NORMAL

    def _keywordCheck(self):
        if self.tok == 'namespace':
            self.mode = Mode.COLLECT_NS_NAME
            self.namespaceType = NamespaceType.NORMAL
        elif self.tok == 'inline':
            self.mode = Mode.COLLECT_NAMESPACE_KEYWORD
        elif self.tok == 'class' or self.tok == 'struct':
            self.mode = Mode.COLLECT_TYPE_NAME
            self.typeType = self.tok
        elif self.tok == 'enum':
            self.mode = Mode.COLLECT_ENUM
            self.typeType = self.tok
        elif self.tok == 'template':
            self.mode = Mode.SKIP_UNTIL_EOL
            self.preTypeLines.append(self.line[self.tokPos:])
        elif self.tok == 'extern':
            # we assume it's a single line extern and simply ignore the content
            self.mode = Mode.SKIP_UNTIL_EOL
        elif self.tok.startswith('#'):
            self.mode = Mode.SKIP_UNTIL_EOL
        elif self.tok.startswith('//'):
            self.mode = Mode.SKIP_UNTIL_EOL
            self.preTypeLines.append(self.line[self.tokPos:])
        elif self.tok == '}':
            self.namespaceStack.pop()
        elif self.tok == '\n':
            pass
        else:
            info('final token', self.tok)
            self.parsingDone = True

    def _getPath(self):
        current = self.declarations
        for namespaceName in self.namespaceStack:
            current = current['namespaces'][namespaceName]
        return current

    def _collectNamespaceKeyword(self):
        if 'namespace' == self.tok:
            self.mode = Mode.COLLECT_NS_NAME
            self.namespaceType = NamespaceType.INLINE
        else:
            info('final token', self.tok)
            self.parsingDone = True

    def _collectNsName(self):
        if not self.namespaceStack:
            self.previousBlockLineNumber = self.lastLineNumber

        if '{' == self.tok:
            info('final token, anon namespace')
            self.parsingDone = True
            return

        current = self._getPath()
        if self.tok not in current['namespaces']:
            current['namespaces'][self.tok] = createDict()
            current['namespaces'][self.tok]['type'] = self.namespaceType

        self.namespaceStack.append(self.tok)
        self.mode = Mode.COLLECT_OPENING_BRACE

        fqNamespace = '::'.join(self.namespaceStack)
        for filtered in FILTER_NAMESPACES:
            if re.match(filtered, fqNamespace):
                break
        else:
            self.lastNamespaceLineNumber = self.lastLineNumber

    def _addForward(self, typeType, typeName):
        if not (self.typeType and self.lastName):
            return

        current = self._getPath()
        if typeName in current['forwards']:
            self.errorReporter(self.NAME, Line(self.path, typeName, self.lastLineNumber))

        current['forwards'][typeName] = {'type': typeType, 'pre': self.preTypeLines}

        self.hadForwards = True
        self.lastForwardLineNumber = self.lastLineNumber
        self.typeType = None
        self.lastName = None
        self.preTypeLines = []

    def _collectOpeningBrace(self):
        self.mode = Mode.NORMAL
        if not '{' == self.tok:
            info('invalid token >>{}<<, expected: {}'.format(self.tok, '{'))
            self.parsingDone = True

    def _collectSemiColon(self):
        self.mode = Mode.NORMAL
        if ';' == self.tok:
            self._addForward(self.typeType, self.lastName)
        else:
            info('invalid token >>{}<<, expected: ;'.format(self.tok))
            self.parsingDone = True

    def _collectTypeName(self):
        self.lastName = self.tok
        self.mode = Mode.COLLECT_SEMICOLON

    def _collectEnum(self):
        if 'class' == self.tok:
            self.typeType = 'enum class'
            return

        if not self.lastName:
            self.lastName = self.tok
            return

        if ':' in self.lastName and self.lastName.endswith(' :'):
            self.lastName += ' {}'.format(self.tok)
            return

        if ':' == self.tok:
            self.lastName += ' {}'.format(self.tok)
            return

        if ';' == self.tok:
            self._collectSemiColon()
        else:
            info('invalid token >>{}<<, expected: ; in >>{} {}<<'.format(self.tok, self.typeType, self.lastName))
            self.parsingDone = True

    def _skipTillEol(self):
        if self.tok == '\n':
            self.mode = Mode.NORMAL

    @staticmethod
    def _formatFwd(className, classSpec):
        formatted = '{} {};'.format(classSpec['type'], className)
        if not classSpec['pre']:
            return [formatted]

        temp = classSpec['pre'][:]
        temp.append(formatted)
        return temp

    @staticmethod
    def indentNotEmpty(line):
        return '\t' + line if line else line

    @staticmethod
    def _format(declarations, level=1):
        result = []
        if declarations['namespaces']:
            # deliberately start from 1, to simplify condition at the bottom of the loop
            for namespaceName, subDeclarations in sorted(declarations['namespaces'].items()):
                lines = ForwardsValidator._format(subDeclarations, level + 1)
                nsType = declarations['namespaces'][namespaceName]['type']
                content = 'inline ' if NamespaceType.INLINE == nsType else ''

                if not lines:
                    continue

                if 1 == level and result:
                    result.append('')

                if len(lines) == 1:
                    content += 'namespace %s { %s }' % (namespaceName, lines[0])
                    result.append(content)

                else:
                    content += 'namespace %s {' % namespaceName
                    result.append(content)
                    indentLines = map(ForwardsValidator.indentNotEmpty, lines)
                    result.extend(indentLines)
                    result.append('}')

        if declarations['forwards']:
            forwards = []
            for className, classSpec in sorted(declarations['forwards'].items(), key=lambda x: x[0].lower()):
                currentForward = ForwardsValidator._formatFwd(className, classSpec)

                # if forward has anything before it (comment or template<>)
                # prepend additional empty line
                if forwards and len(currentForward) > 1:
                    forwards.append('')
                forwards.extend(currentForward)
            result.extend(forwards)

        return result

    def finalize(self):
        reportForwardsError = True
        for pattern in SKIP_FORWARDS:
            if re.match(pattern, self.path):
                reportForwardsError = False
                break

        if self.lastNamespaceNextLineLength and self.lastNamespaceLineNumber > self.lastForwardLineNumber:
            self.errorReporter(self.NAME, Line(self.path, '', self.lastNamespaceLineNumber + 1, 'missingEmptyLine'))

        if reportForwardsError and self.hadForwards:
            expectedLines = self._format(self.declarations)
            expected = '\n'.join(expectedLines)
            current = '\n'.join(self.collectedLines[:len(expectedLines)])
            if current != expected:
                self.errorReporter(self.NAME, Line(self.path, current, (self.matchLineNumber, self.previousBlockLineNumber - 1), expected))

    @staticmethod
    def formatError(err):
        if isinstance(err.lineno, tuple):
            name = err.path
            errorFmt = '{}:{} - {} forward declarations mismatch, has:\n{}\nshould be:\n{}'
            return errorFmt.format(name, err.lineno[0], err.lineno[1], err.line, err.kind)

        if err.kind == 'missingEmptyLine':
            name = err.path
            errorFmt = '{}:{} - missing empty line after namespace'
            return errorFmt.format(name, err.lineno, err.line)

        name = err.path
        errorFmt = '{}:{} - redefinition of a type forward declaration: {}'
        return errorFmt.format(name, err.lineno, err.line)
