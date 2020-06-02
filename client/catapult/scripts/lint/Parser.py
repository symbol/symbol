from collections import defaultdict
from enum import Enum
import re
import sys
import traceback
from xml.sax.saxutils import escape as xmlEscape

from colorPrint import warning, colorPrint, Fore, Style
from cppLexer import *  # pylint: disable=wildcard-import,unused-wildcard-import
from validation import Line

lex.lex()


class Mode(Enum):
    Normal = 1
    CollectNs = 2
    CollectTemplate = 3
    CollectClass = 4
    CollectEnum = 5
    InsideClassOrEnum = 6
    FindSemiColon = 7
    FindClosingParen = 8
    FindClosingBrace = 9
    Operator = 10


class NextTokenBehavior(Enum):
    Skip = 1
    Pick = 2


PRINT_INFO = 0
PRINT_DEBUG = 0
PRINT_TRACE = 0

ANON_NS_FAKENAME = '<anon>'
TEXT_OUTPUT = False
DEST_DIR = '.'


def info(*args):
    if 1 == PRINT_INFO:
        colorPrint(Fore.GREEN, *args)


def debug(*args):
    if 1 == PRINT_DEBUG:
        colorPrint(Fore.GREEN, *args)


def trace_print(*args):
    if 1 == PRINT_TRACE:
        print(*args)


def has(tokenName, stack):
    for token in stack:
        if token.type == tokenName:
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
        self.hadForward = self.hadForward or other.hadForward
        self.hadFuncOrVar = self.hadFuncOrVar or other.hadFuncOrVar
        self.hadClass = self.hadClass or other.hadClass
        self.hadEnum = self.hadEnum or other.hadEnum
        self.hadUsing = self.hadUsing or other.hadUsing
        self.hadTest = self.hadTest or other.hadTest
        self.hadConstant = self.hadConstant or other.hadConstant
        self.hadInclude = self.hadInclude or other.hadInclude
        # this is a hack for some catapult-specific macros usage
        self.hadDefineMacro = self.hadDefineMacro or other.hadDefineMacro
        return self

    def empty(self):
        return not (self.hadForward or
                    self.hadClass or
                    self.hadEnum or
                    self.hadFuncOrVar or
                    self.hadUsing or
                    self.hadTest or
                    self.hadConstant or
                    self.hadInclude or
                    self.hadDefineMacro)

    def __eq__(self, other):
        return self.name == other.name

    def __hash__(self):
        return hash(self.name)

    def __repr__(self):
        return '{} (fwd:{}, class:{}, enum:{}, funcOrVar:{}, using:{}, const:{} inc:{}, def:{})'.format(
            self.name or 'None',
            self.hadForward,
            self.hadClass,
            self.hadEnum,
            self.hadFuncOrVar,
            self.hadUsing,
            self.hadConstant,
            self.hadInclude,
            self.hadDefineMacro)


# pylint: disable=too-many-instance-attributes
class NamespacesParser:
    def __init__(self, errorReporter, path):
        self.errorReporter = errorReporter
        self.path = path
        self.lineNumber = 0
        self.mode = Mode.Normal
        self.curNsPart = ''
        self.namespaceStack = []
        self.templateBracketLevel = 0
        self.templateHadClass = False
        self.templateContent = []
        self.wasTemplateInstantiation = False
        self.hadParens = False
        self.currentBraceLevel = 0
        self.currentColonBraceLevel = 0
        self.nameStack = []
        self.currentParenLevel = 0
        self.closingBraceCallback = None
        self.namespaces = set([])
        self.templateErrors = []
        self.insideTemplateCallback = None

        self.tok = None
        info(self.path)
        self.parseFile(open(self.path, 'r'))

    def _quitIfNoNamestack(self, token):
        if not self.nameStack:
            self.quit(token)

    def parseNormal(self, tok):
        if tok.type == 'CLOSE_BRACE':
            self.addNamespace()
            return

        if tok.type in ('COLON', 'ASTERISK', 'COMMA', 'AMPERSTAND', 'OPEN_SQUARE_BRACKET', 'CLOSE_SQUARE_BRACKET'):
            self.nameStack.append(tok)
            return

        dispatch = {
            'NAME': self._parseNormalName,
            'STRING_LITERAL': self._parseNormalStringLiteral,
            'OPEN_BRACKET': self._parseNormalOpenBracket,
            'PRECOMP_MACRO': self._parseNormalPrecompMacro,
            'OPEN_PAREN': self._parseNormalOpenParen,
            'OPEN_BRACE': self._parseNormalBrace,
            'SEMI_COLON': self._parseNormalSemiColon,
            'EQUALS': self._parseNormalEquals
        }

        if tok.type in dispatch:
            dispatch[tok.type](tok)
        else:
            self.quit(tok)

    def _parseNormalName(self, tok):
        if tok.value == 'namespace':
            self.mode = Mode.CollectNs
            self.curNsPart = ''
            debug('Collect NS')
        elif tok.value == 'template':
            self.insideTemplateCallback = None
            self.templateContent = []
            self.collectTemplate(tok)
            self.mode = Mode.CollectTemplate
            debug('Collect Template')
        elif tok.value == 'class' or tok.value == 'struct':
            self.mode = Mode.CollectClass
            self.nameStack.append(tok)
            debug('Collect ' + tok.value)
        elif tok.value == 'enum':
            self.mode = Mode.CollectEnum
            self.nameStack.append(tok)
            debug('Collect Enum')
        elif tok.value == 'using':
            self.mode = Mode.FindSemiColon
            self.nameStack.append(tok)
            debug('Collect Using')
        else:
            self.nameStack.append(tok)
            if tok.value == 'operator':
                self.mode = Mode.Operator

    def _parseNormalStringLiteral(self, tok):
        # Ignore extern "C"
        if not tok.value == '"C"':
            self.quit(tok)

    def _parseNormalOpenBracket(self, tok):
        self.insideTemplateCallback = lambda tok: self.nameStack.append(tok)  # pylint: disable=unnecessary-lambda
        self.templateContent = []
        self.collectTemplate(tok)
        self.mode = Mode.CollectTemplate

    def _parseNormalPrecompMacro(self, tok):
        if self.namespaceStack and tok.value.startswith('#include'):
            self.namespaceStack[-1].hadInclude = True
            info('HAD INCLUDE')

        if not self.namespaceStack and re.match(r'#define [A-Z_]*TEST_CLASS ', tok.value):
            errorMsg = '`#define TEST_CLASS` outside of any namespace'
            self.errorReporter('preprocessorOther', Line(self.path, '', tok.lineno, errorMsg))

    def _parseNormalOpenParen(self, tok):
        debug('open paren')
        self._quitIfNoNamestack(tok)

        self.findCloseParen(tok)
        self.mode = Mode.FindClosingParen

    def _parseNormalBrace(self, tok):
        self._quitIfNoNamestack(tok)

        self.findCloseBrace(tok)
        trace_print('level brace open', self.currentBraceLevel, 'parens', self.hadParens)
        self.mode = Mode.FindClosingBrace
        if self.hadParens or self.nameStack[0].value == 'extern':
            self.closingBraceCallback = self.clearNameStack
        else:
            self.closingBraceCallback = self.clearNameStackAndFindSemiColon

    def _parseNormalSemiColon(self, tok):
        self._quitIfNoNamestack(tok)

        if has('OPEN_PAREN', self.nameStack) and has('CLOSE_PAREN', self.nameStack):
            if self.namespaceStack:
                self.namespaceStack[-1].hadFuncOrVar = True
                info('HAD FUNC or VAR')

        self.clearNameStack()

    def _parseNormalEquals(self, tok):
        self._quitIfNoNamestack(tok)
        self.nameStack.append(tok)
        self.mode = Mode.FindSemiColon

    def saveTokenOrBye(self, previousToken, token, tokenName):
        del previousToken
        if token.type == tokenName:
            self.nameStack.append(token)
        else:
            self.quit(token)

    def _operatorEquals(self, nextToken):
        if nextToken.type == 'EQUALS':
            self.nameStack.append(nextToken)
            return NextTokenBehavior.Pick

        # operator=
        self.tok = nextToken
        return NextTokenBehavior.Skip

    def _operatorPlus(self, nextToken):
        # operator+= operator++
        if nextToken.type == 'EQUALS' or nextToken.type == 'PLUS':
            self.nameStack.append(nextToken)
            return NextTokenBehavior.Pick

        # operator+
        self.tok = nextToken
        return NextTokenBehavior.Skip

    def _operatorMinus(self, nextToken):
        # operator-= operator-- operator->
        if nextToken.type == 'EQUALS' or nextToken.type == 'MINUS' or nextToken.type == 'CLOSE_BRACKET':
            self.nameStack.append(nextToken)
            return NextTokenBehavior.Pick

        # operator-
        self.tok = nextToken
        return NextTokenBehavior.Skip

    def _operatorLessThan(self, nextToken):
        # operator <<, <=
        if nextToken.type == 'OPEN_BRACKET' or nextToken.type == 'EQUALS':
            self.nameStack.append(nextToken)
            return NextTokenBehavior.Pick

        # operator<
        self.tok = nextToken
        return NextTokenBehavior.Skip

    def _operatorGreaterThan(self, nextToken):
        # operator >=
        if nextToken.type == 'EQUALS':
            self.nameStack.append(nextToken)
            return NextTokenBehavior.Pick

        # operator>
        self.tok = nextToken
        return NextTokenBehavior.Skip

    # pylint: disable=too-many-branches
    def collectOperator(self):
        tok = lex.token()
        self.nameStack.append(tok)
        tok2 = lex.token()
        behavior = NextTokenBehavior.Pick
        # operator()
        if tok.type == 'OPEN_PAREN':
            self.saveTokenOrBye(tok, tok2, 'CLOSE_PAREN')
        # operator[]
        elif tok.type == 'OPEN_SQUARE_BRACKET':
            self.saveTokenOrBye(tok, tok2, 'CLOSE_SQUARE_BRACKET')
        # operator==
        elif tok.type == 'EQUALS':
            behavior = self._operatorEquals(tok2)
        elif tok.type == 'PLUS':
            behavior = self._operatorPlus(tok2)
        elif tok.type == 'MINUS':
            behavior = self._operatorMinus(tok2)
        # operator!=
        elif tok.type == 'EXCLAMATION':
            self.saveTokenOrBye(tok, tok2, 'EQUALS')
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
            behavior = self._operatorLessThan(tok2)
        elif tok.type == 'CLOSE_BRACKET':
            behavior = self._operatorGreaterThan(tok2)
        else:
            self.quit(tok2)

        if behavior == NextTokenBehavior.Pick:
            self.tok = lex.token()

    def addNamespace(self):
        name = '::'.join(map(lambda c: c.current, self.namespaceStack))
        if ANON_NS_FAKENAME in name:
            if self.path.endswith('.h'):
                self.errorReporter('anonNamespace', Line(self.path, name, self.tok.lineno))

        current = self.namespaceStack.pop()
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

    def addTemplateError(self, line):
        name = '::'.join(map(lambda c: c.current, self.namespaceStack))
        dummy = TemplateError(name, line)
        self.templateErrors.append(dummy)

    def clearNameStack(self):
        self.nameStack = []
        self.closingBraceCallback = None

    def clearNameStackAndFindSemiColon(self):
        self.tok = lex.token()
        self.nameStack.append(self.tok)
        if self.tok.type == 'SEMI_COLON':
            self.switchToNormal()
            self.checkProperties()
        else:
            self.quit(self.tok)

        self.nameStack = []
        self.closingBraceCallback = None

    def parseFile(self, inputStream):
        lex.input(inputStream.read())
        while True:
            if self.mode == Mode.Operator:
                self.collectOperator()
                self.mode = Mode.Normal
            else:
                self.tok = lex.token()
                trace_print(self.mode, self.tok)
            if not self.tok:
                break
            dispatch = {
                Mode.Normal: self.parseNormal,
                Mode.CollectNs: self.collectNs,
                Mode.CollectTemplate: self.collectTemplate,
                Mode.CollectClass: self.collectClass,
                Mode.CollectEnum: self.collectEnum,
                Mode.InsideClassOrEnum: self.findClassEnd,
                Mode.FindSemiColon: self.findSemiColon,
                Mode.FindClosingParen: self.findCloseParen,
                Mode.FindClosingBrace: self.findCloseBrace
            }
            dispatch[self.mode](self.tok)

    def quit(self, tok):
        if not TEXT_OUTPUT:
            with open(DEST_DIR + '/tests.fatalerror.xml', 'w') as outputStream:
                outputStream.write('<?xml version="1.0" encoding="UTF-8"?>\n')
                outputStream.write('<testsuites tests="0" failures="0" disabled="0" errors="1" time="0" name="AllTests">\n')
                outputStream.write('  <testsuite name="Parser" tests="0" failures="0" disabled="0" errors="1" time="0">\n')
                msg = 'Error while parsing the file: {}\n'.format(self.path)
                for _ in range(20):
                    msg += repr(tok.type) + ' : ' + repr(tok.value) + '\n'
                    tok = lex.token()
                    if not tok:
                        break

                outputStream.write('  <testcase status="run" time="0" classname="parser" name="ERROR">\n')
                outputStream.write('    <error message="{}" type=""><![CDATA[{}]]></error>\n'.format(xmlEscape(msg), msg))
                outputStream.write('  </testcase>\n')
                outputStream.write('  </testsuite>\n')
                outputStream.write('</testsuites>\n')

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

    def switchToNormal(self):
        self.mode = Mode.Normal
        self.hadParens = False

    def collectNs(self, tok):
        if tok.type == 'NAME':
            self.curNsPart = tok.value
        elif tok.type == 'OPEN_BRACE':
            if not self.curNsPart:
                self.curNsPart = ANON_NS_FAKENAME
            namespace = NamespaceInfo(self.curNsPart)
            self.namespaceStack.append(namespace)
            self.switchToNormal()
        elif tok.type == 'EQUALS':
            self.mode = Mode.FindSemiColon
            debug('got namespace rename "{}"'.format(self.curNsPart))

    def collectTemplate(self, tok):
        self.templateContent.append(tok)
        if tok.type == 'OPEN_BRACKET':
            self.templateBracketLevel += 1
        elif tok.type == 'CLOSE_BRACKET':
            self.templateBracketLevel -= 1
            if self.templateBracketLevel == 0:
                if self.templateHadClass:
                    line = ' '.join(map(lambda e: e.value, self.templateContent))
                    self.addTemplateError(line)
                    self.templateHadClass = False

                if self.wasTemplateInstantiation:
                    self.mode = Mode.FindSemiColon
                    self.wasTemplateInstantiation = False
                else:
                    self.switchToNormal()

        # template instantiation
        elif tok.type == 'NAME' and (tok.value == 'class' or tok.value == 'struct'):
            if self.templateBracketLevel > 0:
                self.templateHadClass = True
            if self.insideTemplateCallback:
                self.insideTemplateCallback(tok)
            self.wasTemplateInstantiation = True
            tok = lex.token()

        if self.insideTemplateCallback:
            self.insideTemplateCallback(tok)

    def collectClass(self, tok):
        if tok.type == 'OPEN_BRACE':
            self.currentBraceLevel += 1
            self.mode = Mode.InsideClassOrEnum
            if self.namespaceStack:
                self.namespaceStack[-1].hadClass = True
                info('HAD CLASS')
            else:
                warning('WARNING: Class at ROOT namespace found: ', self.path)

        elif tok.type == 'SEMI_COLON':
            self.switchToNormal()

            # do not consider forward declarations in global scope
            # (they do not add anything to namespace)
            if not self.namespaceStack:
                # no need to clearNameStack() cause it is empty
                return

            self.namespaceStack[-1].hadForward = True

            name = '::'.join(map(lambda c: c.current, self.namespaceStack))
            objName = ' '.join(map(lambda c: c.value, self.nameStack))
            info('HAD FORWARD', name, objName)
            self.clearNameStack()
        else:
            self.nameStack.append(tok)

    def collectEnum(self, tok):
        if tok.type == 'OPEN_BRACE':
            self.currentBraceLevel += 1
            self.mode = Mode.InsideClassOrEnum
            if self.namespaceStack:
                self.namespaceStack[-1].hadEnum = True
                info('HAD ENUM')
            else:
                warning('WARNING: Enum at ROOT namespace found: ', self.path)

        elif tok.type == 'SEMI_COLON':
            self.switchToNormal()
            self.namespaceStack[-1].hadForward = True

            name = '::'.join(map(lambda c: c.current, self.namespaceStack))
            objName = ' '.join(map(lambda c: c.value, self.nameStack))
            info('HAD FORWARD', name, objName)
            self.clearNameStack()

        else:
            self.nameStack.append(tok)

    def findClassEnd(self, tok):
        if self.currentBraceLevel == 0:
            if tok.type == 'SEMI_COLON':
                self.switchToNormal()
                self.clearNameStack()
            else:
                self.quit(tok)
        if tok.type == 'OPEN_BRACE':
            self.currentBraceLevel += 1
        if tok.type == 'CLOSE_BRACE':
            self.currentBraceLevel -= 1

    def findCloseBrace(self, tok):
        if tok.type == 'OPEN_BRACE':
            if 0 == self.currentBraceLevel:
                self.nameStack.append(tok)
            self.currentBraceLevel += 1
            trace_print('level brace open', self.currentBraceLevel)
        elif tok.type == 'CLOSE_BRACE':
            self.currentBraceLevel -= 1
            trace_print('level brace close', self.currentBraceLevel)
            if self.currentBraceLevel == 0:
                self.nameStack.append(tok)
                self.switchToNormal()
                self.checkProperties()
                self.closingBraceCallback()

    def _checkFuncOrVar(self):
        if has('OPEN_BRACE', self.nameStack) and has('CLOSE_BRACE', self.nameStack):
            if self.namespaceStack:
                self.namespaceStack[-1].hadFuncOrVar = True
                info('HAD FUNC or VAR')
            else:
                functionName = ' '.join(map(lambda e: e.value, self.nameStack))
                info('WARNING: Probably function at ROOT namespace found: ', self.path, functionName)
        if has('OPEN_SQUARE_BRACKET', self.nameStack) and has('CLOSE_SQUARE_BRACKET', self.nameStack):
            if self.namespaceStack:
                self.namespaceStack[-1].hadFuncOrVar = True
                info('HAD FUNC or VAR')
            else:
                warning('WARNING: Unknown case: ', self.path, ' '.join(map(lambda e: e.value, self.nameStack)))

    def checkProperties(self):
        if self.nameStack[0].value == 'using':
            if self.namespaceStack:
                self.namespaceStack[-1].hadUsing = True
                info('HAD USING')
        elif self.nameStack[0].value == 'TEST':
            if self.namespaceStack:
                self.namespaceStack[-1].hadTest = True
                info('HAD TEST')
        elif self.nameStack[0].value == 'extern':
            # Ignore
            pass
        else:
            self._checkFuncOrVar()

            # if constexpr and has assignment
            if self.nameStack[0].value == 'constexpr' and has('EQUALS', self.nameStack):
                self.namespaceStack[-1].hadConstant = True
                info('HAD Constant')

    def findSemiColon(self, tok):
        if self.currentColonBraceLevel == 0:
            self.nameStack.append(tok)
            if tok.type == 'SEMI_COLON':
                self.switchToNormal()
                self.checkProperties()
                self.clearNameStack()

        if tok.type == 'OPEN_BRACE':
            self.currentColonBraceLevel += 1
            trace_print('level colon brace open', self.currentColonBraceLevel)
        if tok.type == 'CLOSE_BRACE':
            self.currentColonBraceLevel -= 1
            trace_print('level colon brace clode', self.currentColonBraceLevel)

    def findCloseParen(self, tok):
        self.nameStack.append(tok)
        if tok.type == 'OPEN_PAREN':
            self.currentParenLevel += 1
            trace_print('level after open ', self.currentParenLevel)
        if tok.type == 'CLOSE_PAREN':
            self.currentParenLevel -= 1
            trace_print('level after close ', self.currentParenLevel)
            if self.currentParenLevel == 0:
                if self.namespaceStack:
                    if self.nameStack and self.nameStack[0].value.startswith('DEFINE_'):
                        self.namespaceStack[-1].hadDefineMacro = True
                        info('HAD DEFINE_* macro')
                self.switchToNormal()
                self.hadParens = True
                debug('close paren {} namespaceStack:{}'.format(self.hadParens, len(self.namespaceStack)))
