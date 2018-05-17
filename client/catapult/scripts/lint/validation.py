import re

from SimpleValidator import SimpleValidator, Line
from forwardsValidation import ForwardsValidator

def stripCommentsAndStrings(line):
    # drop inline comments, remove strings and comments
    temp = re.sub(r'//.*', '', line)
    temp = re.sub(r'/\*(.+?)\*/', 'dummy', temp)
    temp = re.sub(r'"(.+?)"', 'dummy', temp)
    temp = re.sub(r"'(.+?)'", 'dummy', temp)
    return temp

# pylint: disable=too-many-instance-attributes
class WhitespaceLineValidator(SimpleValidator):
    SUITE_NAME = 'Whitespaces'
    NAME = 'whitespaceLines'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternWhitespaces = re.compile(r'[^\s]\s+$')
        self.patternSpacesStart = re.compile(r'\t* +')
        self.patternTabsStart = re.compile(r'^\t+$')
        self.patternSpacesMiddle = re.compile(r'  +')
        self.patternSpaceOperator = re.compile(r'\(([!]) ')
        self.patternCommentSingle = re.compile(r'//')
        self.patternTabInside = re.compile(r'\S\t')
        self.patternCarriageReturn = re.compile(r'\r')
        self.patternComma = re.compile(r',[^ ]')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.carriageReturnCount = 0

    def check(self, lineNumber, line):
        if re.search(self.patternWhitespaces, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Whitespace at line ending'))
        if re.match(self.patternSpacesStart, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces at beginning of a line'))
        if re.match(self.patternTabsStart, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Tabs in empty line'))

        operatorMatch = re.search(self.patternSpaceOperator, line)
        if operatorMatch:
            errorMsg = 'Space after operator >>{}<<'.format(operatorMatch.group(1))
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))
        if re.search(self.patternTabInside, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Tab present inside the text'))

        if re.search(self.patternSpacesMiddle, line) or re.search(self.patternComma, line):
            temp = stripCommentsAndStrings(line)
            if re.search(self.patternSpacesMiddle, temp):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces in the middle'))

            gotComma = re.search(self.patternComma, temp)
            if gotComma and gotComma.group(0) not in [",)"]:
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Comma should be followed by a space'))

        if re.search(self.patternCarriageReturn, line):
            self.carriageReturnCount += 1

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

    def finalize(self):
        if 0 < self.carriageReturnCount:
            errorMsg = 'Carriage returns present in file {} occurences'.format(self.carriageReturnCount)
            self.errorReporter(self.NAME, Line(self.path, '', 0, errorMsg))

class TestClassMacroValidator(SimpleValidator):
    SUITE_NAME = 'TestClassMacro'
    NAME = 'testClassMacro'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternTestClass = re.compile(r'#define [A-Z_]*TEST_CLASS ')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.lineTestClass = None
        self.matchLineNumber = 0

    def match(self, line, lineNumber):
        hasMatch = re.search(self.patternTestClass, line)
        if hasMatch:
            self.lineTestClass = line
            self.matchLineNumber = lineNumber
        return hasMatch

    def check(self, lineNumber, line):
        if self.matchLineNumber and self.matchLineNumber + 1 == lineNumber and line:
            if not self.match(line, lineNumber):
                self.errorReporter(self.NAME, Line(self.path, self.lineTestClass, self.matchLineNumber))

        self.match(line, lineNumber)

    @staticmethod
    def formatError(err):
        name = err.path
        errMsg = '{}:{} TEST_CLASS should be followed by an empty line: >>{}<<'
        return errMsg.format(name, err.lineno, err.line)

class LineLengthValidator(SimpleValidator):
    SUITE_NAME = 'LongLines'
    NAME = 'tooLongLines'

    def __init__(self, errorReporter, lineLengthLimit=140):
        super().__init__(errorReporter)
        self.lineLengthLimit = lineLengthLimit

    def check(self, lineNumber, line):
        temp = re.sub('\t', '    ', line)
        if len(temp) >= self.lineLengthLimit:
            self.errorReporter(self.NAME, Line(self.path, line.strip(), lineNumber))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} Line too long: >>{}<<'.format(name, err.lineno, err.line)

class TemplateSpaceValidator(SimpleValidator):
    SUITE_NAME = 'Template'
    NAME = 'templateFollowedBySpace'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternTemplate = re.compile(r'template\s+<')

    def check(self, lineNumber, line):
        if re.search(self.patternTemplate, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Template followed by space'))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class CatchWithoutClosingTryBrace(SimpleValidator):
    SUITE_NAME = 'Catch Formatting'
    NAME = 'catchAndClosingTryBraceOnSeparateLines'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternTemplate = re.compile(r'^\s+catch')

    def check(self, lineNumber, line):
        if re.search(self.patternTemplate, line):
            errorMsg = 'catch and closing try brace must be on same line'
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class SingleLineValidator(SimpleValidator):
    SUITE_NAME = 'SingleLine'
    NAME = 'singleLine'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.numOpen = 0
        self.firstLine = 0
        self.long = ''

    def check(self, lineNumber, line):
        if self.numOpen:
            stripped = line.lstrip()
            if stripped.startswith('//'):
                self.numOpen = 0
                return

            self.long = self.long + stripped
            if self.long.endswith(','):
                self.long += ' '

            temp = stripCommentsAndStrings(stripped)

            numBrackets = 0
            hadBrace = False
            for char in temp:
                if '(' == char:
                    self.numOpen += 1
                elif ')' == char:
                    self.numOpen -= 1
                elif '[' == char or ']' == char:
                    numBrackets += 1
                elif '{' == char:
                    hadBrace = True

            if hadBrace and numBrackets > 1:
                self.numOpen = 0
                return

            if 0 >= self.numOpen:
                temp = re.sub('\t', '    ', self.long)
                if len(temp) < 140:
                    errorMsg = 'block fits in a single line'
                    self.errorReporter(self.NAME, Line(self.path, self.long, self.firstLine, errorMsg))

        # find lines ending with ( - possible function calls or definitions/declarations
        # but ignore raw string literals
        if line.endswith('(') and not line.endswith('R"('):
            self.numOpen = 1
            self.firstLine = lineNumber
            self.long = line

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class PragmaOnceValidator(SimpleValidator):
    SUITE_NAME = 'Pragmas'
    NAME = 'pragmaErrors'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        # only .h files need pragma once
        self.gotPragmaOnce = None if path.endswith('.h') else True
        self.gotLicense = False
        self.emptyLineNumber = 0
        self.reportEmptyLineError = None if path.endswith('.h') else False
        self.insideComment = 0

    def check(self, lineNumber, line):
        # detect header notice and skip
        if line.startswith('/**'):
            self.insideComment = 1
            self.gotLicense = True

        if 1 == self.insideComment:
            if '**/' in line:
                self.insideComment = 2
            return
        # used to skip empty line after closing comment
        elif 2 == self.insideComment:
            self.insideComment = 3
            return

        if line.startswith('#include'):
            # we don't want empty line between pragma and include
            if self.reportEmptyLineError is None:
                self.reportEmptyLineError = 0 < self.emptyLineNumber

        if line.startswith('#'):
            if self.reportEmptyLineError is None:
                self.reportEmptyLineError = False

        if self.gotPragmaOnce:
            if not line:
                self.emptyLineNumber = lineNumber

        if self.gotPragmaOnce is None:
            self.gotPragmaOnce = True if line == '#pragma once' else False

    @staticmethod
    def formatError(err):
        name = err.path
        return '{} {}'.format(name, err.kind)

    def finalize(self):
        if not self.gotLicense:
            self.errorReporter(self.NAME, Line(self.path, '', 0, 'Missing license info'))

        if not self.gotPragmaOnce:
            self.errorReporter(self.NAME, Line(self.path, '', 0, 'Missing `#pragma once`'))

        if self.reportEmptyLineError:
            self.errorReporter(self.NAME, Line(self.path, '', self.emptyLineNumber, 'Empty line after `#pragma once`'))


class TypoChecker(SimpleValidator):
    SUITE_NAME = 'Typos'
    NAME = 'nameTypo'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.errors = {
            re.compile(r'imeStamp'): 'Timestamp not TimeStamp',
            re.compile(r'ileSystem'): 'Filesystem not FileSystem',
            re.compile(r'ileName'): 'Filename not FileName',
            re.compile(r'ile_Name'): 'Filename not File_Name',
            re.compile(r'lockchain'): 'BlockChain not Blockchain',
            re.compile(r'_EQ\(nullptr,'): 'use _FALSE(!!ptr) instead',
            re.compile(r'_NE\(nullptr,'): 'use _TRUE(!!ptr) instead',
            re.compile(r'[!=]= nullptr|nullptr [!=]='): 'don\'t compare against nullptr directly',
            re.compile(r'{ +}'): 'don\'t leave space between braces `{}`',
            re.compile(r'///.*(triggered by |validator implementation).* notification[^s]'): 'use plural notification*S*',
            re.compile(r'document{}'): 'use document()',
            re.compile(r'txElement'): 'use transactionElement',
            re.compile(r'txInfo'): 'use transactionInfo',
            re.compile(r'[^a-zA-Z]pCopy'): 'use pFooCopy instead of pCopyFoo',
            re.compile(r'\scopy[A-Z].*\='): 'use fooCopy instead of copyFoo',
            re.compile(r'operator new \('): 'no space after new',
            re.compile(r'\)override'): 'missing space before override',
            re.compile(r'[^}][^ ]else {'): 'missing brace before else',
            # mind that this pattern matches only simple case where ctor is in a single line
            re.compile(r'explicit\s+\w+\(\)'): 'no-arg ctor should not be explicit',
            re.compile(r'constexpr char (.*)\[\] = "'): 'use constexpr auto ... for constexpr strings',
            re.compile(r'using T[A-Z].* ='): 'do not start aliases with T followed by upper case letter',
            re.compile(r'using Base ='): 'use BaseType instead of Base in alias declaration',
            re.compile(r'return{'): 'missing space after return followed by array or object',
            re.compile(r'#define TEST_CLASS TEST_CLASS'): 'invalid define',
            # note this will miss more complicated expressions like if (!foobar().size())
            re.compile(r'\(!\w+\.size\(\)\)'): 'use .empty() rather than `!.size()`',
            re.compile(r' it( =|;)'): 'use `iter` not it',
            re.compile(r'auto& iter ='): 'seems like invalid use of iterator',
            re.compile(r'->\s+.*[^&]\s+\{'): 'don\'t specify lambda return types for non-reference results',
            re.compile(r'\s(?P<first>.+) \(?\\a (?P=first)\)'): 'simplify documentation: foo (\\a foo) => \\a foo',
            re.compile(r'infos? = \S*TransactionInfo'): 'variable should probably be named *transactionInfo(s)',
            re.compile(r'TransactionInfos?&?&?>? (m_)?info'): 'variable should probably be named utInfo(s)/transactionInfo(s)',
            re.compile(r'\b(pt|cosignedTransaction)Info\b'): 'variable has deprecated transactionInfo name',
            re.compile(r'\b0x[0-9]*[a-f]'): 'use uppercase hex constants',
            re.compile(r'\b0X[0-9A-F]'): 'use `0x` no `0X`',
            re.compile(r'const char\* (m_)?p[A-Z]'): 'const chars should not have pointer naming convention',
            re.compile(r'\.string\(\)'): 'prefer usage of generic_string() instead',
            re.compile(r'EXPECT.*(!|0, )(std::)?memcmp'): 'prefer `EXPECT_TRUE(0 == std::memcmp`',
            re.compile(r'.+Information [: a-zA-Z]*{'): 'use *Info, rather than Information',
            re.compile(r'.+Config [: a-zA-Z]*{'): 'use *Configuration, rather than Config',
            re.compile(r'utils::\w+Raw(Buffer|String)[^;]'): '*RawBuffer, *RawString types are included via types.h, drop utils::',
            re.compile(r'SocketOperationCode&'): 'SocketOperationCode should be passed by value',
            re.compile(r'(SocketOperationCode\s+\w+esult|esult.*=.*SocketOperationCode|[Cc]ode = \w+esult\w*;)'):
                'SocketOperationCode should be named *code not result',
            re.compile(r'#include "test/'): 'do not use local test includes, use fully qualified path',
            re.compile(r'[^,] ,'): 'do not have space before comma',
            re.compile(r'Noop'): 'use NoOp* instead of Noop',
            re.compile(r'#define .*MAKE_.*TESTS'): 'use DEFINE_ for group of tests',
            re.compile(r'hutdowns'): 'use shuts down instead of shutdowns',
            re.compile(r'[cC]ataputl'): 'catapult not cataputl',
            re.compile(r'(\(auto&,|, auto&[,)])'): 'use `const auto&` for non-referred lambda arguments',
            re.compile(r'(\d|0x[0-9a-fA-F]+)u\)'): 'no need for explicit unsigned qualifier',
            re.compile(r';;$'): 'no double semicolons',
            re.compile(r'[a-zA-Z>\*]>[^&\n]*= {'): 'prefer container initialization to container assign',
            re.compile(r'(/\*+|///) The '): 'documentation should not start with \'The\''
        }

    def check(self, lineNumber, line):
        for k, errorMsg in self.errors.items():
            if re.search(k, line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} TYPO {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class BasicFunctionAliasValidator(SimpleValidator):
    """Validates proper usage of templates: supplier<>, action<>, consumer<> and predicate<>"""

    SUITE_NAME = 'FunctionAlias'
    NAME = 'functionAlias'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.errors = {
            re.compile(r'std::function<.*\(\)>'): 'use supplier alias',
            re.compile(r'std::function<void'): 'use action or consumer alias',
            re.compile(r'consumer<>'): 'use action alias',
            re.compile(r'std::function<bool'): 'use predicate alias',
        }

    def check(self, lineNumber, line):
        if re.match(r'src.catapult.functions.h', self.path):
            return

        for k, errorMsg in self.errors.items():
            if re.search(k, line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} TYPO {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)


class SpaceBraceValidator(SimpleValidator):
    """Validates there is no space before brace, if intended usage is variable or object uniform initialization"""

    SUITE_NAME = 'SpaceBrace'
    NAME = 'spaceBrace'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternNameSpaceBrace = re.compile(r'([a-zA-Z][a-zA-Z0-9:<>_]+) +{')
        self.skip = [
            # skip if line contains 'namespace', that line shouldn't be interesting
            re.compile(r'namespace'),

            # skip if it looks like method/function/lambda definition
            re.compile(r'(const|final|override|noexcept|mutable) {'),

            # sikp if it looks like lambda
            re.compile(r']\(.*\)( mutable)? -> [^\r\n\t\f]+ {'),

            # skip if it looks like inheritance
            re.compile(r'[:,] (public|protected|private) .+'),

            # skip if it looks like type definition
            re.compile(r'(struct|class|enum) \w+'),

            # skip different blocks
            # last one - we do have some anon union
            re.compile(r'\W(try|else|do|union) {'),

            # skip labels
            re.compile(r'^\s+case .*:'),

            # skip comments
            re.compile(r'^\s*//'),

            #####################
            ## special exceptions

            # we have single occurance like this
            re.compile(r'enum : '),

            # special for src/catapult/utils/Logging.h
            re.compile(r'custom_info_tagger<SubcomponentTraits>>>'),
        ]

    def check(self, lineNumber, line):
        # we want to match things like Foo {
        # and things like std::set<Foo> {
        result = re.search(self.patternNameSpaceBrace, line)
        if not result:
            return

        # skip if it matched return
        if 'return' == result.group(1):
            return

        for pattern in self.skip:
            if re.search(pattern, line):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, result.group(1)))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} Space between type or variable >>{}<< and brace: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class ReturnOnNewLineValidator(SimpleValidator):
    """Validates that return is placed in a separate line"""

    SUITE_NAME = 'ReturnOnNewLine'
    NAME = 'returnOnNewLine'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternReturn = re.compile(r'\S\s*return .*;')
        self.skip = [
            # skip if it looks like lambda
            re.compile(r']\(.*\)( mutable)? -> [^\r\n\t\f]+ {'),
            re.compile(r'\[.*\]\(.*\)( mutable)? { return'),

            re.compile(r'static constexpr .* return'),

            # skip comments
            re.compile(r'^\s*//'),
        ]

    def check(self, lineNumber, line):
        result = re.search(self.patternReturn, line)
        if not result:
            return

        for pattern in self.skip:
            if re.search(pattern, line):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} `return` should be on newline >>{}<<'.format(name, err.lineno, err.line)

class MultiConditionChecker(SimpleValidator):
    """Validator for more complicated cases, that require several conditions.

    * tests use `TEST(TEST_CLASS` construction
    * `operator bool` if overridden is always `explicit`
    * in validator tests, if `ValidationResult` is last argument, argument name should be `value`
    * single argument ctor is explicit
    * enums are always scoped (enum class)
    """

    SUITE_NAME = 'MultiConditionChecker'
    NAME = 'multiConditionChecker'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternOperatorBool = re.compile(r'operator bool')
        self.patternOperatorBoolWithExplicit = re.compile(r'explicit operator bool')
        self.patternTest = re.compile(r'\s+(NO_STRESS_)?TEST\(')
        self.patternTestClass = re.compile(r'TEST\([A-Z_]*TEST_CLASS')
        self.patternValidationResult = re.compile(r',\s+ValidationResult\s+(\w+)\)')
        self.isTestValidator = False
        self.patternMissingExplicitCtor = re.compile(r'^\s+(\w+)\((?:const )?(\w+)[&]? \w+\);$')
        self.patternEnum = re.compile(r'enum [^:]')
        self.patternEnumClass = re.compile(r'enum class ')
        self.patternCoerce = re.compile(r'\s+(.*)auto(.).*= (ionet::)?CoercePacket')
        self.patternDefineTests = re.compile(r'#define [A-Z_]*DEFINE_[A-Z_]*_TEST[^S]')
        self.patternDefineTestTraits = re.compile(r'#define [A-Z_]*DEFINE_[A-Z_]*_TEST_TRAITS[A-Z_]*')

    def reset(self, path):
        super().reset(path)
        # match common part of validation, validator
        self.isTestValidator = re.search(r'validat', path, re.IGNORECASE) and re.search(r'test', path, re.IGNORECASE)

    def checkExplicitOperatorBool(self, line):
        return re.search(self.patternOperatorBool, line) and not re.search(self.patternOperatorBoolWithExplicit, line)

    def checkTestLine(self, line):
        # special file, skip it
        if self.path.endswith('Stress.h'):
            return False

        # if doesn't contain TEST( it's not interesting at all
        if not re.search(self.patternTest, line):
            return False

        # if contains TEST(TEST_CLASS, mark as ok
        if re.search(self.patternTestClass, line):
            return False

        # macros redefining TEST should use TEST_NAME
        if re.search(r'TEST_NAME', line):
            return False

        # if it contains preprocessor concatenation, it means it's probably inside some other #define
        if re.search(r'##', line):
            return False

        # mark as invalid
        return True

    def checkExplicitCtor(self, line):
        match = re.match(self.patternMissingExplicitCtor, line)
        if not match:
            return False

        # plugins\txes\multisig\src\validators\Validators.h contains false positives, due to long definitions
        if match.group(1).startswith('Create'):
            return False

        # this implies copy ctor `Foo(const Foo& ...)`
        if match.group(1) == match.group(2):
            return False

        return True

    def checkValidationResult(self, line):
        if not self.isTestValidator:
            return False

        match = re.search(self.patternValidationResult, line)
        if match:
            return match.group(1) != 'value'

        return False

    def checkEnumClass(self, line):
        return re.search(self.patternEnum, line) and not re.search(self.patternEnumClass, line)

    def checkCoerce(self, line):
        match = re.search(self.patternCoerce, line)
        if match:
            return match.group(1) != 'const ' or match.group(2) != '*'

        return False

    def checkDefineTests(self, line):
        return re.match(self.patternDefineTests, line) and not re.match(self.patternDefineTestTraits, line)

    def check(self, lineNumber, line):
        strippedLine = stripCommentsAndStrings(line)
        # note that these checks are exclusive
        if self.checkTestLine(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'TEST should use TEST_CLASS'))
        elif self.checkExplicitOperatorBool(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Missing explicit before operator bool'))
        elif self.checkValidationResult(strippedLine):
            msg = 'ValidationResult should not be last argument or should be called `value`'
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, msg))
        elif self.checkExplicitCtor(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'missing explicit before ctor'))
        elif self.checkEnumClass(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'use enum class instead of enum'))
        elif self.checkCoerce(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'use const auto* .. = CoercePacket'))
        elif self.checkDefineTests(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'use MAKE_ for singular TEST'))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

def rindex(lst, searched):
    return next(idx for idx, value in zip(range(len(lst)-1, -1, -1), reversed(lst)) if value == searched)

class UtilsSubdirValidator(SimpleValidator):
    """Validates there is no `utils` subdir under tests"""

    SUITE_NAME = 'UtilsSubdir'
    NAME = 'utilsSubdir'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.hasUtils = False
        splitted = re.split(r'[/\\]', path)
        if 'tests' in splitted and 'utils' in splitted:
            idx = rindex(splitted, 'utils')
            # skip catapult/utils
            if idx > 0 and splitted[idx - 1] != 'catapult':
                self.hasUtils = True

    def check(self, lineNumber, line):
        pass

    def finalize(self):
        if self.hasUtils:
            self.errorReporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{} utils subdirectory should be replaced with test >>{}<<'.format(name, err.line)

class RegionValidator(SimpleValidator):
    """Validates nested regions and region typos"""

    SUITE_NAME = 'RegionValidator'
    NAME = 'regionValidator'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternRegion = re.compile(r'//(.*)region')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.counter = 0
        self.previousRegionLine = None
        self.previousRegionLineNumber = 0
        self.firstBeforeNestedLine = None
        self.firstBeforeNestedLineNumber = 0
        self.errors = []

    def check(self, lineNumber, line):
        result = re.search(self.patternRegion, line)
        if not result:
            return

        prefix = result.group(1)
        if prefix == ' ':
            if self.counter > 0:
                if not self.firstBeforeNestedLine:
                    self.firstBeforeNestedLine = self.previousRegionLine
                    self.firstBeforeNestedLineNumber = self.previousRegionLineNumber
                msg = 'nested region (top-most in line:{})'.format(self.firstBeforeNestedLineNumber)
                self.errors.append(Line(self.path, line.strip('\n\r'), lineNumber, msg))

            self.previousRegionLine = line.strip('\n\r')
            self.previousRegionLineNumber = lineNumber
            self.counter += 1
        elif prefix == ' end':
            if self.counter == 0:
                self.errors.append(Line(self.path, line.strip('\n\r'), lineNumber, 'endregion without corresponding region'))

            self.counter -= 1
        else:
            # if name is invalid report immediatelly
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'invalid region "{}"'.format(result.group(0))))

    def finalize(self):
        if self.counter > 0:
            line = self.firstBeforeNestedLine or self.previousRegionLine
            lineNumber = self.firstBeforeNestedLineNumber or self.previousRegionLineNumber
            self.errorReporter(self.NAME, Line(self.path, line, lineNumber, 'non-closed region (probable location)'))

        elif self.errors:
            for error in self.errors:
                self.errorReporter(self.NAME, error)

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class MacroSemicolonValidator(SimpleValidator):
    """Validator for ensuring that macros don't have trailing semicolons."""

    SUITE_NAME = 'MacroSemicolonChecker'
    NAME = 'macroSemicolonChecker'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.macroCall = re.compile(r'^\s+[A-Z_]+\([^\)]*\);')

        # exclude special macros that should have semicolons some or most of the time
        self.skip = [
            re.compile(r'EXPECT_|ASSERT_'), # test asserts
            re.compile(r'CATAPULT_'), # catapult macros (LOG, THROW)
            re.compile(r'WAIT_FOR'), # wait for macros
            re.compile(r'LOAD_([A-Z]+_)*PROPERTY'), # property loading macros
            re.compile(r'DEFINE_[A-Z]+_NOTIFICATION'), # notification definition macros
            re.compile(r'DEFINE_([A-Z]+_)+RESULT'), # result definition macros
            re.compile(r'DEFINE_(ENTITY|TRANSACTION|NOTIFICATION)_TYPE'), # entity and notification type definition macros
            re.compile(r'DECLARE_MONGO_CACHE_STORAGE'), # macros that can be used for function declarations
            re.compile(r'DEFINE_MOCK_(FLUSH|INFOS)_CAPTURE'), # macros used for mock class definitions
        ]

    def check(self, lineNumber, line):
        strippedLine = stripCommentsAndStrings(line)

        if not re.match(self.macroCall, strippedLine):
            return

        for pattern in self.skip:
            if re.search(pattern, strippedLine):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'macro should not have trailing semicolon'))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class EnumValueBlankLineValidator(SimpleValidator):
    """Validator for ensuring that ENUM_VALUE entries are followed by blank lines."""

    SUITE_NAME = 'EnumValueBlankLineChecker'
    NAME = 'enumValueBlankLineChecker'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternEnumValue = re.compile(r'\s*ENUM_VALUE\(.*\) \\')
        self.patternBlankLine = re.compile(r'\s*\\')
        self.previousLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r')

        if re.match(self.patternEnumValue, self.previousLine) and not re.match(self.patternBlankLine, strippedLine):
            self.errorReporter(self.NAME, Line(self.path, self.previousLine, lineNumber - 1, 'enum value is missing following blank line'))

        self.previousLine = strippedLine

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)


def createValidators(errorReporter):
    validators = [
        WhitespaceLineValidator,
        TestClassMacroValidator,
        LineLengthValidator,
        TemplateSpaceValidator,
        CatchWithoutClosingTryBrace,
        SingleLineValidator,
        PragmaOnceValidator,
        TypoChecker,
        BasicFunctionAliasValidator,
        SpaceBraceValidator,
        ForwardsValidator,
        ReturnOnNewLineValidator,
        MultiConditionChecker,
        UtilsSubdirValidator,
        RegionValidator,
        MacroSemicolonValidator,
        EnumValueBlankLineValidator
    ]
    return list(map(lambda validator: validator(errorReporter), validators))
