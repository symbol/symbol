import re
from enum import Enum

from colorPrint import colorPrint, Fore
from cppLexer import * #pylint: disable=wildcard-import,unused-wildcard-import
from SimpleValidator import SimpleValidator, Line
from exclusions import SKIP_FORWARDS

PRINT_INFO = False
def info(*args):
    if 1 == PRINT_INFO:
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
    Normal = 1
    CollectNsName = 2
    CollectOpeningBrace = 3
    CollectTypeName = 4
    CollectSemiColon = 5
    SkipTillEol = 6

def createDict():
    return {'namespaces':{}, 'forwards':{}}


# pylint: disable=too-many-instance-attributes
class ForwardsValidator(SimpleValidator):
    SUITE_NAME = 'ForwardDeclaration'
    NAME = 'forward'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternNamespace = re.compile(r'\s*namespace')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.matchLineNumber = 0
        self.parsingDone = False
        self.mode = Mode.Normal
        self.tokenizer = SimpleNsTokenizer()
        self.tok = None
        self.namespaceStack = []
        self.declarations = createDict()
        self.typeType = None
        self.lastName = None
        self.preTypeLines = []
        self.hadForwards = False
        self.lastLineNumber = None
        self.previousBlockLineNumber = None
        self.collectedLines = []

        for pattern in SKIP_FORWARDS:
            if re.match(pattern, path):
                self.parsingDone = True

    def check(self, lineNumber, line):
        if 0 == self.matchLineNumber:
            result = re.match(self.patternNamespace, line)
            if not result:
                return
            self.matchLineNumber = lineNumber
        self._parse(lineNumber, line)

    def _parse(self, lineNumber, line):
        if self.parsingDone:
            return

        self.lastLineNumber = lineNumber
        self.collectedLines.append(line)

        self.tokenizer.feed(line + '\n')
        self.line = line
        dispatch = {
            Mode.Normal: self._keywordCheck,
            Mode.CollectNsName: self._collectNsName,
            Mode.CollectOpeningBrace: self._collectOpeningBrace,
            Mode.CollectTypeName: self._collectTypeName,
            Mode.CollectSemiColon: self._collectSemiColon,
            Mode.SkipTillEol: self._skipTillEol
        }
        for tok in self.tokenizer:
            if self.parsingDone:
                return

            self.tokPos = tok[0]
            self.tok = tok[1]
            dispatch[self.mode]()

        if self.mode == Mode.SkipTillEol:
            self.mode = Mode.Normal

    def _keywordCheck(self):
        if self.tok == 'namespace':
            self.mode = Mode.CollectNsName
        elif self.tok == 'class' or self.tok == 'struct':
            self.mode = Mode.CollectTypeName
            self.typeType = self.tok
        elif self.tok == 'template':
            self.mode = Mode.SkipTillEol
            self.preTypeLines.append(self.line[self.tokPos : ])
        elif self.tok.startswith('#'):
            self.mode = Mode.SkipTillEol
        elif self.tok.startswith('//'):
            self.mode = Mode.SkipTillEol
            self.preTypeLines.append(self.line[self.tokPos : ])
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

        self.namespaceStack.append(self.tok)
        self.mode = Mode.CollectOpeningBrace

    def _addForward(self, typeType, typeName):
        if not (self.typeType and self.lastName):
            return

        current = self._getPath()
        if typeName in current['forwards']:
            self.errorReporter(self.NAME, Line(self.path, typeName, self.lastLineNumber))

        current['forwards'][typeName] = {'type': typeType, 'pre': self.preTypeLines}

        self.hadForwards = True
        self.typeType = None
        self.lastName = None
        self.preTypeLines = []

    def _collectOpeningBrace(self):
        self.mode = Mode.Normal
        if not '{' == self.tok:
            info('invalid token >>{}<<, expected: {}'.format(self.tok, '{'))
            self.parsingDone = True

    def _collectSemiColon(self):
        self.mode = Mode.Normal
        if ';' == self.tok:
            self._addForward(self.typeType, self.lastName)
        else:
            info('invalid token >>{}<<, expected: ;'.format(self.tok))
            self.parsingDone = True

    def _collectTypeName(self):
        self.lastName = self.tok
        self.mode = Mode.CollectSemiColon

    def _skipTillEol(self):
        if self.tok == '\n':
            self.mode = Mode.Normal

    @staticmethod
    def _formatFwd(className, classSpec):
        formatted = '{} {};'.format(classSpec['type'], className)
        if not classSpec['pre']:
            return [formatted]

        temp = classSpec['pre'][:]
        temp.append(formatted)
        return temp

    @staticmethod
    def identNonEmpty(line):
        return '\t' + line if line else line

    @staticmethod
    def _format(declarations):
        result = []
        if declarations['namespaces']:
            for namespaceName, subDeclarations in sorted(declarations['namespaces'].items()):
                lines = ForwardsValidator._format(subDeclarations)
                if not lines:
                    pass
                elif len(lines) == 1:
                    result.append('namespace %s { %s }' % (namespaceName, lines[0]))

                else:
                    result.append('namespace %s {' % namespaceName)
                    indentLines = map(ForwardsValidator.identNonEmpty, lines)
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
        if self.hadForwards:
            current = '\n'.join(self.collectedLines[: (self.previousBlockLineNumber - 1 - self.matchLineNumber)])
            expected = '\n'.join(self._format(self.declarations))
            if current != expected:
                self.errorReporter(self.NAME, Line(self.path, current, (self.matchLineNumber, self.previousBlockLineNumber - 1), expected))

    @staticmethod
    def formatError(err):
        if isinstance(err.lineno, tuple):
            name = err.path
            errorFmt = '{}:{} - {} forward declarations mismatch, has:\n{}\nshould be:\n{}'
            return errorFmt.format(name, err.lineno[0], err.lineno[1], err.line, err.kind)

        name = err.path
        errorFmt = '{}:{} - redefinition of a type forward declaration: {}'
        return errorFmt.format(name, err.lineno, err.line)
