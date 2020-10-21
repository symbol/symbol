# pylint: disable=too-many-lines
from hashlib import sha1
from binascii import unhexlify
import re

from SimpleValidator import SimpleValidator, Line
from forwardsValidation import ForwardsValidator


def stripCommentsAndStrings(line):
    # drop inline comments, remove strings and comments
    temp = re.sub(r'//.*', '', line)
    temp = re.sub(r'/\*(.+?)\*/', 'dummy', temp)
    temp = re.sub(r'"(.+?)"', 'dummy', temp)
    temp = re.sub('\'(.+?)\'', 'dummy', temp)
    return temp


# pylint: disable=too-many-instance-attributes
class WhitespaceLineValidator(SimpleValidator):
    SUITE_NAME = 'Whitespaces'
    NAME = 'whitespaceLines'

    def __init__(self):
        super().__init__()
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
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.carriageReturnCount = 0

    def check(self, lineNumber, line):
        if self.patternWhitespaces.search(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Whitespace at line ending'))
        if self.patternSpacesStart.match(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces at beginning of a line'))
        if self.patternTabsStart.match(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Tabs in empty line'))

        operatorMatch = self.patternSpaceOperator.search(line)
        if operatorMatch:
            errorMsg = 'Space after operator >>{}<<'.format(operatorMatch.group(1))
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))
        if self.patternTabInside.search(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Tab present inside the text'))

        if self.patternSpacesMiddle.search(line) or self.patternComma.search(line):
            temp = stripCommentsAndStrings(line)
            if self.patternSpacesMiddle.search(temp):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces in the middle'))

            gotComma = self.patternComma.search(temp)
            if gotComma and gotComma.group(0) not in [',)']:
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Comma should be followed by a space'))

        if self.patternCarriageReturn.search(line):
            self.carriageReturnCount += 1

    def finalize(self):
        if 0 < self.carriageReturnCount:
            errorMsg = 'Carriage returns present in file {} occurences'.format(self.carriageReturnCount)
            self.errorReporter(self.NAME, Line(self.path, '', 0, errorMsg))


class TestClassMacroValidator(SimpleValidator):
    SUITE_NAME = 'TestClassMacro'
    NAME = 'testClassMacro'

    def __init__(self):
        super().__init__()
        self.patternTestClass = re.compile(r'#define [A-Z_]*TEST_CLASS (\s+)')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.lineTestClass = None
        self.matchLineNumber = 0
        self.hasMismatchedTestClass = False

        splitted = re.split(r'[/\\]', path)
        self.filename = splitted[-1]

    def match(self, line, lineNumber):
        hasMatch = self.patternTestClass.search(line)
        if hasMatch:
            self.lineTestClass = line
            self.matchLineNumber = lineNumber

            # some files have multiple test class macros, so just check that they end with the filename
            if not (hasMatch.group(1) + '.cpp').endswith(self.filename):
                self.hasMismatchedTestClass = True

        return hasMatch

    def check(self, lineNumber, line):
        if self.matchLineNumber and self.matchLineNumber + 1 == lineNumber and line:
            if not self.match(line, lineNumber):
                self.errorReporter(self.NAME, Line(self.path, self.lineTestClass, self.matchLineNumber))

        self.match(line, lineNumber)

    def finalize(self):
        if self.hasMismatchedTestClass:
            self.errorReporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def formatError(err):
        name = err.path
        errMsg = '{}:{} TEST_CLASS should be followed by an empty line and match filename: >>{}<<'
        return errMsg.format(name, err.lineno, err.line)


class LineLengthValidator(SimpleValidator):
    SUITE_NAME = 'LongLines'
    NAME = 'tooLongLines'

    def __init__(self, lineLengthLimit=140):
        super().__init__()
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

    def __init__(self):
        super().__init__()
        self.patternTemplate = re.compile(r'template\s+<')

    def check(self, lineNumber, line):
        if self.patternTemplate.search(line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Template followed by space'))


class CatchWithoutClosingTryBrace(SimpleValidator):
    SUITE_NAME = 'Catch Formatting'
    NAME = 'catchAndClosingTryBraceOnSeparateLines'

    def __init__(self):
        super().__init__()
        self.patternTemplate = re.compile(r'^\s+catch')

    def check(self, lineNumber, line):
        if self.patternTemplate.search(line):
            errorMsg = 'catch and closing try brace must be on same line'
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))


class SingleLineValidator(SimpleValidator):
    SUITE_NAME = 'SingleLine'
    NAME = 'singleLine'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
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
                elif char in '[]':
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


class PragmaOnceValidator(SimpleValidator):
    SUITE_NAME = 'Pragmas'
    NAME = 'pragmaErrors'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
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
        if 2 == self.insideComment:
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
            self.gotPragmaOnce = line == '#pragma once'

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

    def __init__(self):
        super().__init__()
        self.errors = {
            re.compile(r'imeStamp|ime [sS]tamp'): 'Timestamp not TimeStamp or Time Stamp',
            re.compile(r'ileSystem|ile [sS]ystem'): 'Filesystem not FileSystem or File System',
            re.compile(r'ile_?Name|ile [nN]ame'): 'Filename not FileName or File_Name or File Name',
            re.compile(r'o[nt]Zero|[nN]o[nt] [zZ]ero'): 'Nonzero not NonZero or Non Zero or NotZero or Not Zero',
            re.compile(r'hreadpool'): 'ThreadPool not Threadpool',
            re.compile(r'lockchain'): 'BlockChain not Blockchain',
            re.compile(r'onEmpty|on-empty'): 'NotEmpty not NonEmpty or non-empty',
            re.compile(r'oundTrip|ound [tT]rip'): 'Roundtrip not RoundTrip or Round Trip',
            re.compile(r'alidatorResult'): 'ValidationResult not ValidatorResult',
            re.compile(r'ub-?cache'): 'SubCache or sub cache not Subcache or sub-cache',
            re.compile(r'on[eE]xisting|onExistent'): 'Nonexistent',
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
            re.compile(r'(EXPECT|ASSERT)_.*end\(\),.*find\('): 'prefer EXPECT_CONTAINS',
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
            re.compile(r'(/\*+|///?) (The|An?) [^=]'): 'documentation should not start with \'The\' or `A(n)`',
            re.compile(r'::(En|Dis)able[^d]'): 'enum values should be named Enable*d/Disable*d',
            re.compile(r', and'): 'rephrase to avoid \', and\'',
            re.compile(r'// #include'): 'don\'t comment out includes!',
            re.compile(r'TId>'): 'use TIdentifier instead of TId',
            re.compile(r'const{'): 'add space between const and brace',
            re.compile(r'CreatePropertyChecker|typename TValidationFunction|builderFactory'): 'avoid this pattern for builder tests',
            re.compile(r', \)'): 'no space after comma',
            re.compile(r'\b(EntityType|ReceiptType|ValidationResult)\((0[xX])?\d+\)'): 'use static_cast instead',
            re.compile(r'~.*\(\)\s*{}'): 'use default instead',
            re.compile(r'EXPECT_EQ\(.*(ransaction|lock|eceipt|uffer|ntity|acket)(s\[\w+\])?(\.|->)\w*Size\);'):
                'looks like size comparison, use ASSERT instead',
            re.compile(r'typedef'): 'prefer using',
            re.compile(r'unsigned char'): 'prefer uint8_t',
            re.compile(r'static_cast<.*>\(\w*::\w*_\w*\)'): 'cpp17 shouldn\'t need suspect cast',
            re.compile(r'constexpr auto \w+\(\) {'): 'cpp17 shouldn\'t need suspect function',
            re.compile(r'^\s*(inline|constexpr)\s*$'): 'combine with following line',
            re.compile(r'(inline|constexpr) static'): 'static first',
            re.compile(r'acquire(Reader|Writer)\(\)(\);|,)'): 'for safety, acquire read lock outside of view constructor',
            re.compile(r'(reader|writer)Lock'): 'prefer readLock/writeLock',
            re.compile(r'createDetachableDelta\(\).detach\(\)'): 'warning: releasing read lock at end of scope, might lead to crash',
            re.compile(r'sizeof\((model::)?Block\)'): 'use sizeof(BlockHeader)',
            re.compile(r'static const [^{]+$'): 'use static constexpr',
            re.compile(r'&>.*\.data\(\)'): 'use [] operator',
            re.compile(r'\bdata[0-9]* =|std::vector<uint8_t> data'): 'prefer buffer',
            re.compile(r' ;$'): 'no space before semicolon',
            re.compile(r'(struct|class) \S+{'): 'add space before {',
            re.compile(r'\b[A-Z]\w+{}'): 'zero initialize with () not \\{\\}',
            re.compile(r'^\s+} }|{ {$'): 'remove space between braces',
            re.compile(r'_tag::'): 'use ByteArray instead of _tag',
            re.compile(r'Observes.*including'): 'use and instead of including',
            re.compile(r'/// (Arrange|Act|Assert):'): 'use //',
            re.compile(r'pair<bool'): 'use pair with bool as second',
            re.compile(r'\ba \\a'): 'just use \\a',
            re.compile(r'\binto \\a builder'): 'into => to',
            re.compile(r'cosigner'): 'cosigner(s) => cosignatory(ies)',
            re.compile(r'^\t*WAIT_FOR_VALUE(_EXPR)?\([01]u,'): 'use _ZERO or _ONE',
            re.compile(r'(EXPECT|ASSERT)_.*(ToHexString|Hex\b)'): 'compare buffers directly',
            re.compile(r'(EXPECT|ASSERT)_.*[^"],[^<]*(ToString)'): 'compare buffers directly (rule 2)',
            re.compile(r'while\('): 'missing space after while',
            re.compile(r'while ?\(0\)'): 'use while (false)',
            re.compile(r'\) do {'): 'start `do` on own line',
            re.compile(r'/// (Return|Set|Get)\b'): 'prefer plural',
            re.compile(r'/// Returns [^\\]'): 'prefer /// Gets for non boolean values',
            re.compile(r'/// Gets \\c (true|false)'): 'prefer /// Returns for boolean values',
            re.compile(r'\\[^c] (true|false)'): 'use \\c for booleans',
            re.compile(r'/// Gets the (const )?(pointer|reference)\b'): 'use a instead of the',
            re.compile(r'\d+u( [^:] \d+)*u'): 'only first `u` is needed',
            re.compile(r' \.([^\.]|$)'): 'check spacing around \'.\'',
            re.compile(r'Header::(Footer|Header)'): 'drop Header',
            re.compile(r'\S \(\)[^>]'): 'remove space before ()',
            re.compile(r'(etwork|ccount)Ids?\d*\b'): 'use Identifier instead of Id',
            re.compile(r'typename T?AccountKey'): 'use TAccountIdentifier',
            re.compile(r'ccountKeys?\b|ccount keys'): 'qualify with public or private',
            re.compile(r'shared_ptr<(thread::)?IoThreadPool'): 'use unique_ptr instead',
            re.compile(r'auto pData\b'): 'use auto*',
            re.compile(r'[^:]memcpy\('): 'use std::memcpy',
            re.compile(r'\)\{$'): 'missing space before brace'
        }

    def check(self, lineNumber, line):
        for k, errorMsg in self.errors.items():
            if k.search(line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))


class BasicFunctionAliasValidator(SimpleValidator):
    """Validates proper usage of templates: supplier<>, action<>, consumer<> and predicate<>"""

    SUITE_NAME = 'FunctionAlias'
    NAME = 'functionAlias'

    def __init__(self):
        super().__init__()
        self.errors = {
            re.compile(r'std::function<.*\(\)>'): 'use supplier alias',
            re.compile(r'std::function<void'): 'use action or consumer alias',
            re.compile(r'consumer<>'): 'use action alias',
            re.compile(r'std::function<bool'): 'use predicate alias'
        }

    def check(self, lineNumber, line):
        if re.match(r'src.catapult.functions.h', self.path):
            return

        for k, errorMsg in self.errors.items():
            if k.search(line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))


class SpaceBraceValidator(SimpleValidator):
    """Validates there is no space before brace, if intended usage is variable or object uniform initialization"""

    SUITE_NAME = 'SpaceBrace'
    NAME = 'spaceBrace'

    def __init__(self):
        super().__init__()
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
            # special exceptions

            # we have single occurrence like this
            re.compile(r'enum : (uint|size_t)'),

            # special for src/catapult/utils/Logging.h
            re.compile(r'custom_info_tagger<SubcomponentTraits>>>')
        ]

    def check(self, lineNumber, line):
        # we want to match things like Foo {
        # and things like std::set<Foo> {
        result = self.patternNameSpaceBrace.search(line)
        if not result:
            return

        # skip if it matched return
        matched = result.group(1)
        if 'return' == matched:
            return

        # skip if only closing angle bracket is used (MultisigCacheTypes)
        if '>' in matched and '<' not in matched:
            return

        for pattern in self.skip:
            if pattern.search(line):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, matched))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} Space between type or variable >>{}<< and brace: >>{}<<'.format(name, err.lineno, err.kind, err.line)


class ReturnOnNewLineValidator(SimpleValidator):
    """Validates that return is placed in a separate line"""

    SUITE_NAME = 'ReturnOnNewLine'
    NAME = 'returnOnNewLine'

    def __init__(self):
        super().__init__()
        self.patternReturn = re.compile(r'\S\s*return .*;')
        self.skip = [
            # skip if it looks like lambda
            re.compile(r']\(.*\)( mutable)? -> [^\r\n\t\f]+ {'),
            re.compile(r'\[.*\]\(.*\)( mutable)? { return'),

            re.compile(r'static constexpr .* return'),

            # skip comments
            re.compile(r'^\s*//')
        ]

    def check(self, lineNumber, line):
        result = self.patternReturn.search(line)
        if not result:
            return

        for pattern in self.skip:
            if pattern.search(line):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} `return` should be on newline >>{}<<'.format(name, err.lineno, err.line)


# pylint: disable=too-many-public-methods
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

    def __init__(self):
        super().__init__()
        self.patternOperatorBool = re.compile(r'[^\:]operator bool')
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
        self.patternFileSize = re.compile(r'FileSize&')
        self.patternOperator = re.compile(r'operator')
        self.patternTryParseValue = re.compile(r'TryParseValue')
        self.patternFileSizeCast = re.compile(r'const_cast<utils::FileSize&>.* = ')
        self.patternTestExpectedSize = re.compile(r'expected\w*Size =[^=]')

        # never allow memcmp in an assert
        self.patternTestMemcmpAssert = re.compile(r'(ASSERT|EXPECT).*memcmp')

        # disallow == and != in an assert unless (1) iterator check with `ASSERT_TRUE(c.end() != iter)` or (2) explicit operators
        self.patternTestBoolAssert = re.compile(r'(ASSERT|EXPECT)_(TRUE|FALSE).*(==|!=)')
        self.patternTestBoolAssertAllowed = re.compile(r'(ASSERT_TRUE.*end\(\) !=)|operator(==|!=)')

        self.patternDeclareMacroNoParams = re.compile(r'DECLARE_(.+_)?(OBSERVER|VALIDATOR).*\(\)')

        self.patternSingleLineFunction = re.compile(r'^[^\[\]]*\) { .*; }')
        self.patternTestSingleLineFunction = re.compile(r'TEST_CLASS|TEST_NAME')

        self.patternTestNameIf = re.compile(r'TEST.*If|\b(Next|Remove)When')
        self.patternTestNameIfExclusions = re.compile(r'\b(Next|Remove)If')

        self.patternHeaderComment = re.compile(r'^[^/]*// .*\.$')
        self.patternDoxygenComment = re.compile(r'///')

        self.patternAutoContextParam = re.compile(r'auto& (context|notification)')

        self.patternGetsSetsDoc = re.compile(r'/// (Gets|Sets) ')
        self.patternGetsSetsDocWithArticle = re.compile(r'/// (Gets|Sets) (a|an|the|all|information)\b')

        self.patternTrailingOperator = re.compile(r' (\+|-|\*|/|%|&|\||^|<<|>>)\s*$')

        self.patternStructAssignment = re.compile(r'^\t*(static )?(const(expr)? )?([a-zA-Z][a-zA-Z0-9:_]+) [a-zA-Z_][a-zA-Z0-9_]+ = {')

        self.errors = {
            self.checkTestLine: 'TEST should use TEST_CLASS',
            self.checkExplicitOperatorBool: 'Missing explicit before operator bool',
            self.checkValidationResult: 'ValidationResult should not be last argument or should be called `value`',
            self.checkExplicitCtor: 'missing explicit before ctor',
            self.checkEnumClass: 'use enum class instead of enum',
            self.checkCoerce: 'use const auto* .. = CoercePacket',
            self.checkDefineTests: 'use MAKE_ for singular TEST',
            self.checkFileSize: 'FileSize should be passed by value',
            self.checkTestExpectedSize: 'first size part should start on own line',
            self.checkTestAsserts: 'use a different EXPECT or ASSERT macro',
            self.checkDeclareMacroNoParams: 'use DEFINE macro',
            self.checkSingleLineFunction: 'reformat info multiple lines',
            self.checkTestNameIf: 'use When instead of If',
            self.checkHeaderComment: '. unexpected in header file comment',
            self.checkCppDoxygenComment: '/// unexpected in cpp file',
            self.checkAutoContextParam: 'use type name instead of auto',
            self.checkGetsSetsDocumentation: 'add an article to documentation',
            self.checkTrailingOperator: 'operators should start lines, not finish them',
            self.checkStructAssignment: 'prefer struct initialization to struct assignment'
        }

    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        # match common part of validation, validator
        self.isTestValidator = re.search(r'validat', path, re.IGNORECASE) and re.search(r'test', path, re.IGNORECASE)

    def checkExplicitOperatorBool(self, line, _):
        return self.patternOperatorBool.search(line) and not self.patternOperatorBoolWithExplicit.search(line)

    def checkTestLine(self, line, _):
        # special file, skip it
        if self.path.endswith('Stress.h'):
            return False

        # if doesn't contain TEST( it's not interesting at all
        if not self.patternTest.search(line):
            return False

        # if contains TEST(TEST_CLASS, mark as ok
        if self.patternTestClass.search(line):
            return False

        # macros redefining TEST should use TEST_NAME
        if re.search(r'TEST_NAME', line):
            return False

        # if it contains preprocessor concatenation, it means it's probably inside some other #define
        if re.search(r'##', line):
            return False

        # mark as invalid
        return True

    def checkExplicitCtor(self, line, _):
        match = self.patternMissingExplicitCtor.match(line)
        if not match:
            return False

        # plugins\txes\multisig\src\validators\Validators.h contains false positives, due to long definitions
        if match.group(1).startswith('Create'):
            return False

        # this implies copy ctor `Foo(const Foo& ...)`
        if match.group(1) == match.group(2):
            return False

        # allow implicit constructors for some types
        if match.group(1) in ['Resolvable', 'TestBlockTransactions']:
            return False

        return True

    def checkValidationResult(self, line, _):
        if not self.isTestValidator:
            return False

        match = self.patternValidationResult.search(line)
        if match:
            return match.group(1) != 'value'

        return False

    def checkEnumClass(self, line, _):
        return self.patternEnum.search(line) and not self.patternEnumClass.search(line)

    def checkCoerce(self, line, _):
        match = self.patternCoerce.search(line)
        if match:
            return match.group(1) != 'const ' or match.group(2) != '*'

        return False

    def checkDefineTests(self, line, _):
        return self.patternDefineTests.match(line) and not self.patternDefineTestTraits.match(line)

    def checkFileSize(self, line, _):
        if not self.patternFileSize.search(line):
            return False

        # ignore operator, TryParseValue
        if self.patternOperator.search(line) or self.patternTryParseValue.search(line):
            return False

        # ignore cast
        if self.patternFileSizeCast.search(line):
            return False

        return True

    def checkTestExpectedSize(self, line, _):
        return self.patternTestExpectedSize.search(line) and ';' not in line

    def checkTestAsserts(self, line, _):
        # special file, skip it
        if self.path.endswith('TestHarness.h'):
            return False

        # fail if memcmp is used in a test assert
        if self.patternTestMemcmpAssert.search(line):
            return True

        # fail if == or != are used in a test assert
        return self.patternTestBoolAssert.search(line) and not self.patternTestBoolAssertAllowed.search(line)

    def checkDeclareMacroNoParams(self, line, _):
        # rule only applies to cpp files
        return self.path.endswith('.cpp') and self.patternDeclareMacroNoParams.search(line)

    def checkSingleLineFunction(self, line, _):
        return self.patternSingleLineFunction.search(line) and not self.patternTestSingleLineFunction.search(line)

    def checkTestNameIf(self, line, _):
        return self.patternTestNameIf.search(line) and not self.patternTestNameIfExclusions.search(line)

    def checkHeaderComment(self, _, rawLine):
        # rule only applies to header files
        return self.path.endswith('.h') and self.patternHeaderComment.match(rawLine)

    def checkCppDoxygenComment(self, _, rawLine):
        # rule only applies to cpp files
        return self.path.endswith('.cpp') and self.patternDoxygenComment.search(rawLine)

    def checkAutoContextParam(self, line, _):
        # rule only applies to observer and validator implementations
        return self.path.endswith(('Observer.cpp', 'Validator.cpp')) and self.patternAutoContextParam.search(line)

    def checkGetsSetsDocumentation(self, _, rawLine):
        return self.patternGetsSetsDoc.search(rawLine) and not self.patternGetsSetsDocWithArticle.search(rawLine)

    def checkTrailingOperator(self, line, _):
        # not part of SimpleValidator because comments and strings should be removed before applying rule
        return self.patternTrailingOperator.search(line)

    def checkStructAssignment(self, line, _):
        match = self.patternStructAssignment.match(line)
        if match:
            # treat auto assignments as valid ones (`auto foo = { ... }`)
            return 'auto' != match.group(4)

        return False

    def check(self, lineNumber, line):
        strippedLine = stripCommentsAndStrings(line)
        for func, errorMsg in self.errors.items():
            if func(strippedLine, line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))


class Cpp17TraitsValidator(SimpleValidator):
    """Validates that cpp17 traits are used in most files"""

    SUITE_NAME = 'Cpp17Traits'
    NAME = 'cpp17Traits'

    # pylint: disable=attribute-defined-outside-init
    def __init__(self):
        super().__init__()
        self.typePattern = re.compile(r'>::type\b')
        self.valuePattern = re.compile(r'>::value\b')

    def check(self, lineNumber, line):
        #  skip custom traits files that need to use ::value and ::type
        if self.path.endswith(('Traits.h', 'StlTraits.h', 'TraitsTests.cpp', 'StlTraitsTests.cpp')):
            return

        errorMessage = ''
        if self.valuePattern.search(line):
            errorMessage = 'use _v instead of ::value'

        #  boost::logging requires one usage of ::type
        if not self.path.endswith('Logging.h') and self.typePattern.search(line):
            errorMessage = 'use _t instead of ::type'

        if not errorMessage:
            return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMessage))


def rindex(lst, searched):
    return next(idx for idx, value in zip(range(len(lst)-1, -1, -1), reversed(lst)) if value == searched)


class UtilsSubdirValidator(SimpleValidator):
    """Validates there is no `utils` subdir under tests"""

    SUITE_NAME = 'UtilsSubdir'
    NAME = 'utilsSubdir'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
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


class StressTestNameValidator(SimpleValidator):
    """Validates that stress tests have approprate names"""

    SUITE_NAME = 'StressTestName'
    NAME = 'stressTestName'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.hasImproperName = False
        splitted = re.split(r'[/\\]', path)
        if 'tests' in splitted:
            # skip validation of test util names
            if 'stress' in splitted and 'test' not in splitted:
                if not splitted[-1].endswith('IntegrityTests.cpp'):
                    self.hasImproperName = True
            else:
                if splitted[-1].endswith('IntegrityTests.cpp'):
                    self.hasImproperName = True

    def check(self, lineNumber, line):
        pass

    def finalize(self):
        if self.hasImproperName:
            self.errorReporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{} all stress tests and only stress tests should end with `IntegrityTests.cpp`>>{}<<'.format(name, err.line)


class RegionValidator(SimpleValidator):
    """Validates nested regions and region typos"""

    SUITE_NAME = 'RegionValidator'
    NAME = 'regionValidator'

    def __init__(self):
        super().__init__()
        self.patternRegion = re.compile(r'//(.*)region')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.counter = 0
        self.previousRegionLine = None
        self.previousRegionLineNumber = 0
        self.firstBeforeNestedLine = None
        self.firstBeforeNestedLineNumber = 0
        self.errors = []

    def check(self, lineNumber, line):
        result = self.patternRegion.search(line)
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


class MacroSemicolonValidator(SimpleValidator):
    """Validator for ensuring that macros don't have trailing semicolons."""

    SUITE_NAME = 'MacroSemicolonChecker'
    NAME = 'macroSemicolonChecker'

    def __init__(self):
        super().__init__()
        self.macroCall = re.compile(r'^\s+[A-Z_]+\([^\)]*\);')

        # exclude special macros that should have semicolons some or most of the time
        self.skip = [
            re.compile(r'EXPECT_|ASSERT_'),  # test asserts
            re.compile(r'CATAPULT_'),  # catapult macros (LOG, THROW)
            re.compile(r'WAIT_FOR'),  # wait for macros
            re.compile(r'LOAD_([A-Z]+_)*PROPERTY'),  # property loading macros
            re.compile(r'DEFINE_([A-Z]+_)+(NOTIFICATION|RECEIPT|RESULT)'),  # definition macros
            re.compile(r'DEFINE_(ENTITY|NOTIFICATION|RECEIPT|TRANSACTION)_TYPE'),  # type definition macros
            re.compile(r'DECLARE_MONGO_CACHE_STORAGE'),  # macros that can be used for function declarations
            re.compile(r'DEFINE_MOCK_(FLUSH|INFOS)_CAPTURE'),  # macros used for mock class definitions
        ]

    def check(self, lineNumber, line):
        strippedLine = stripCommentsAndStrings(line)

        if not self.macroCall.match(strippedLine):
            return

        for pattern in self.skip:
            if pattern.search(strippedLine):
                return

        self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'macro should not have trailing semicolon'))


class EnumValueBlankLineValidator(SimpleValidator):
    """Validator for ensuring that ENUM_VALUE entries are followed by blank lines."""

    SUITE_NAME = 'EnumValueBlankLineChecker'
    NAME = 'enumValueBlankLineChecker'

    def __init__(self):
        super().__init__()
        self.patternEnumValue = re.compile(r'\s*ENUM_VALUE\(.*\) \\')
        self.patternBlankLine = re.compile(r'\s*\\')
        self.previousLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r')

        if self.patternEnumValue.match(self.previousLine) and not self.patternBlankLine.match(strippedLine):
            self.errorReporter(self.NAME, Line(self.path, self.previousLine, lineNumber - 1, 'enum value is missing following blank line'))

        self.previousLine = strippedLine


class TrailingCommaValidator(SimpleValidator):
    """Validator for ensuring that there are no trailing commas."""

    SUITE_NAME = 'TrailingCommaChecker'
    NAME = 'trailingCommaChecker'

    def __init__(self):
        super().__init__()
        self.previousLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t')  # strip tabs too

        if self.previousLine and ',' == self.previousLine[-1]:
            if strippedLine and strippedLine[0] in [')', ']', '}']:
                self.errorReporter(self.NAME, Line(self.path, self.previousLine, lineNumber - 1, 'delete trailing comma'))

        self.previousLine = strippedLine


class ClosingBraceVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring that closing braces are properly followed by whitespace."""

    SUITE_NAME = 'ClosingBraceVerticalSpacingChecker'
    NAME = 'closingBraceVerticalSpacingVChecker'

    def __init__(self):
        super().__init__()
        self.patternClosingBrace = re.compile(r'^\s*}\s*$')
        self.patternInbetweenClosingBrace = re.compile(r'^\s*}[}\)]*;?\s*$')
        self.patternLineAfterClosingBrace = re.compile(r'^#|^\s*(}[}\)]*[;,]?|namespace .*|break;|} catch.*|} else.*|])$')
        self.recentLines = None

    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.recentLines = ['', '', '']

    def check(self, lineNumber, line):
        self.recentLines.append(line.strip('\n\r'))
        self.recentLines.pop(0)

        # check if line following brace is valid:
        # 1. preprocessor directive
        # 2. another closing brace (with optional closing symbols and/or semicolon and/or comma)
        # 3. namespace statement within forward declarations
        # 4. break statement within switch
        # 5. catch statement
        # 6. closing square bracket (inline JSON)
        if self.patternClosingBrace.match(self.recentLines[1]):
            if self.recentLines[2] and not self.patternLineAfterClosingBrace.match(self.recentLines[2]):
                self.errorReporter(self.NAME, Line(self.path, self.recentLines[2], lineNumber, 'improper line following closing brace'))

        # check if line between closing braces is valid:
        # 1. blank
        # 2. valid line following brace
        if self.patternClosingBrace.match(self.recentLines[2]) and self.patternClosingBrace.match(self.recentLines[0]):
            if not self.recentLines[1] or not self.patternLineAfterClosingBrace.match(self.recentLines[1]):
                self.errorReporter(self.NAME, Line(self.path, self.recentLines[1], lineNumber - 1, 'improper line between closing braces'))


class NamespaceOpeningBraceVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring that opening namespaces are properly followed by whitespace or not."""

    SUITE_NAME = 'NamespaceOpeningBraceVerticalSpacingChecker'
    NAME = 'namespaceOpeningBraceVerticalSpacingChecker'

    def __init__(self):
        super().__init__()
        self.patternNamespaceOpening = re.compile(r'^(\s*(inline )?namespace \w+ {)+$')
        self.patternAnonNamespaceOpening = re.compile(r'^\s*namespace {')
        self.patternForwardDeclaration = re.compile(r'^\s*(namespace \w+ { )*(class|struct) \w+;')
        self.recentLines = None

    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.recentLines = ['', '', '']

    def check(self, lineNumber, line):
        self.recentLines.append(line.strip('\n\r'))
        self.recentLines.pop(0)

        isOuterNamespace = False
        isInnerOrAnonNamespace = False
        if self.patternNamespaceOpening.match(self.recentLines[1]):
            if '\t' != self.recentLines[1][0]:
                isOuterNamespace = True
            else:
                isInnerOrAnonNamespace = True
        elif self.patternAnonNamespaceOpening.match(self.recentLines[1]):
            isInnerOrAnonNamespace = True

        # check line following outer namespace opening
        followingLine = self.recentLines[2]
        if isOuterNamespace and followingLine:
            if not (self.patternNamespaceOpening.match(followingLine) or self.patternForwardDeclaration.match(followingLine)):
                self.reportError(lineNumber, 'line following namespace opening must be blank or namespace opening or forward declaration')

        # check line following inner or anon namespace opening
        if isInnerOrAnonNamespace and not followingLine:
            self.reportError(lineNumber, 'line following anon or inner namespace opening must not be blank')

    def reportError(self, lineNumber, message):
        self.errorReporter(self.NAME, Line(self.path, self.recentLines[1], lineNumber - 1, message))


class CopyrightCommentValidator(SimpleValidator):
    """Validator for ensuring copyright comment consistency."""

    SUITE_NAME = 'CopyrightCommentChecker'
    NAME = 'copyrightCommentChecker'

    def __init__(self):
        super().__init__()
        self.expectedHash = unhexlify('6d244ea9972afcb6b17d695594958de6dc162f50')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, errorReporter):
        super().reset(path, errorReporter)
        self.hasher = sha1()
        self.lastLineNumber = 0

    def check(self, lineNumber, line):
        self.lastLineNumber = lineNumber
        if lineNumber <= 20:
            self.hasher.update(bytes(line, 'utf-8'))

        if lineNumber == 20:
            if self.expectedHash != self.hasher.digest():
                self.errorReporter(self.NAME, Line(self.path, '', 1))

    def finalize(self):
        if 20 > self.lastLineNumber:
            self.errorReporter(self.NAME, Line(self.path, '', 1))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} invalid copyright comment'.format(name, err.lineno)


class EmptyStatementValidator(SimpleValidator):
    """Validator for ensuring empty statements are formatted correctly."""

    # 1. for empty objects (detect trailing semicolon) with no continuations:
    # class Bar {};
    #
    # 2. for empty destructors and while loops:
    # virtual ~Bar() {}
    # while (cond) {}
    #
    # 3. for other empty statments (detect no trailing semicolon):
    # void Bar()
    # {}

    SUITE_NAME = 'EmptyStatementChecker'
    NAME = 'emptyStatementChecker'

    def __init__(self):
        super().__init__()
        self.isOpeningBraceUnclosed = False
        self.previousStrippedLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t')  # also strip tabs

        # `{}` must be on its own line expect in (2) cases
        if strippedLine.endswith(r'{}') and strippedLine != r'{}':
            if not any(strippedLine.startswith(prefix) for prefix in ['virtual ~', 'while ']):
                self.reportError(lineNumber, line, 'empty statement body must be on new line')

        # `{};` must never be on its own line unless preceeded by what looks like class continuation
        if strippedLine.endswith(r'{};'):
            if any(self.previousStrippedLine.startswith(prefix) or strippedLine.startswith(prefix) for prefix in [':', ',']):
                if strippedLine != r'{};':
                    self.reportError(lineNumber, line, 'empty statement body must be on new line ')
            else:
                if strippedLine == r'{};':
                    self.reportError(lineNumber, line, 'empty statement body must be part of previous line ')

        self.previousStrippedLine = strippedLine

        # mark opening statements
        if strippedLine.endswith(r'{'):
            self.isOpeningBraceUnclosed = True
            return

        if not self.isOpeningBraceUnclosed:
            return

        # empty statements are never allowed to be multiline
        self.isOpeningBraceUnclosed = False
        if strippedLine == r'};':
            self.reportError(lineNumber, line, 'empty statement body must be part of previous line')

        if strippedLine == r'}':
            self.reportError(lineNumber, line, 'empty statement body must be on new line')

    def reportError(self, lineNumber, line, message):
        self.errorReporter(self.NAME, Line(self.path, line, lineNumber, message))


class DocumentationVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring documentation has appropriate vertical spacing."""

    SUITE_NAME = 'DocumentationVerticalSpacingChecker'
    NAME = 'documentationVerticalSpacingChecker'

    def __init__(self):
        super().__init__()
        self.previousStrippedLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t')  # also strip tabs

        if strippedLine.startswith('///'):
            if (self.previousStrippedLine
                    and all(not self.previousStrippedLine.startswith(postfix) for postfix in ['///', '#'])
                    and all(not self.previousStrippedLine.endswith(postfix) for postfix in [':', '{'])):
                self.reportError(lineNumber, line, 'documentation has unexpected previous line')

        self.previousStrippedLine = strippedLine

    def reportError(self, lineNumber, line, message):
        self.errorReporter(self.NAME, Line(self.path, line, lineNumber, message))


class InsertionOperatorFormattingValidator(SimpleValidator):
    """Validator for ensuring insertion operator has appropriate formatting."""

    SUITE_NAME = 'InsertionOperatorFormattingChecker'
    NAME = 'insertionOperatorFormattingChecker'

    def __init__(self):
        super().__init__()
        self.previousStrippedLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t;')  # also strip tabs and semicolons

        if strippedLine.startswith('<<'):
            if (any(strippedLine.endswith(postfix) for postfix in ['open_array', 'open_document'])
                    and self.previousStrippedLine.endswith('"')):
                self.reportError(lineNumber, strippedLine, '<< should be on previous line')

            if self.previousStrippedLine.startswith('<<'):
                return

            if '<<' in self.previousStrippedLine:
                self.reportError(lineNumber - 1, self.previousStrippedLine, '<< should be on own line')

        self.previousStrippedLine = strippedLine

    def reportError(self, lineNumber, line, message):
        self.errorReporter(self.NAME, Line(self.path, line, lineNumber, message))


class TestFirstLineCommentValidator(SimpleValidator):
    """Validator for ensuring that first comments of test functions are well formed."""

    SUITE_NAME = 'TestFirstLineCommentChecker'
    NAME = 'testFirstLineCommentChecker'

    def __init__(self):
        super().__init__()
        self.patternTestFunction = re.compile(r'TEST\(')
        self.patternEndsAssertComment = re.compile(r'//( -|.*Assert:$)')
        self.previousStrippedLine = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t')  # also strip tabs

        if self.patternTestFunction.search(self.previousStrippedLine):
            if self.patternEndsAssertComment.match(strippedLine):
                self.reportError(lineNumber, line, 'should remove redundant assert comment')

        self.previousStrippedLine = strippedLine

    def reportError(self, lineNumber, line, message):
        self.errorReporter(self.NAME, Line(self.path, line, lineNumber, message))


class ExplicitCtorValidator(SimpleValidator):
    """Validator for ensuring that only single argument constructors have explicit keyword."""

    SUITE_NAME = 'ExplicitCtorChecker'
    NAME = 'explicitCtorChecker'

    def __init__(self):
        super().__init__()
        self.patternExplicitCtor = re.compile(r'(constexpr )?explicit \w+\(')

        self.ctorLineNumber = 0
        self.ctorStartLine = ''

        self.openParenCount = 0
        self.openAngleBracketCount = 0
        self.ctorArguments = ''

    def check(self, lineNumber, line):
        strippedLine = line.strip('\n\r\t')  # also strip tabs

        if 0 == self.openParenCount and not self.patternExplicitCtor.match(strippedLine):
            return

        if 0 == self.openParenCount:
            # save for error reporting
            self.ctorLineNumber = lineNumber
            self.ctorStartLine = strippedLine

        for char in strippedLine:
            if '<' == char:
                self.openAngleBracketCount += 1
            elif '>' == char:
                self.openAngleBracketCount -= 1
            if '(' == char:
                self.openParenCount += 1
            elif ')' == char:
                self.openParenCount -= 1
                if 0 == self.openParenCount:
                    self.checkExplicitCtorLine()
                    self.ctorArguments = ''
                    break
            else:
                if 0 == self.openAngleBracketCount and 0 != self.openParenCount:
                    self.ctorArguments += char

    def checkExplicitCtorLine(self):
        if not self.ctorArguments:
            self.reportError('empty ctor should not use explicit')

        # single arg ctor can always be explicit
        splitArgs = self.ctorArguments.split(',')
        if 1 == len(splitArgs):
            return

        # if second arg has default, explicit is allowed
        if any(defaultMarker in splitArgs[1] for defaultMarker in ['=', '...']):
            return

        self.reportError('multi arg ctor should not use explicit')

    def reportError(self, message):
        self.errorReporter(self.NAME, Line(self.path, self.ctorStartLine, self.ctorLineNumber, message))


def createValidators():
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
        Cpp17TraitsValidator,
        UtilsSubdirValidator,
        StressTestNameValidator,
        RegionValidator,
        MacroSemicolonValidator,
        EnumValueBlankLineValidator,
        TrailingCommaValidator,
        ClosingBraceVerticalSpacingValidator,
        NamespaceOpeningBraceVerticalSpacingValidator,
        CopyrightCommentValidator,
        EmptyStatementValidator,
        DocumentationVerticalSpacingValidator,
        InsertionOperatorFormattingValidator,
        TestFirstLineCommentValidator,
        ExplicitCtorValidator
    ]
    return list(map(lambda validator: validator(), validators))
