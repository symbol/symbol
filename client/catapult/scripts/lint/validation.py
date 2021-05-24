# pylint: disable=too-many-lines
import re
from binascii import unhexlify
from hashlib import sha1

from forwardsValidation import ForwardsValidator
from SimpleValidator import Line, SimpleValidator


def strip_comments_and_strings(line):
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
        self.pattern_whitespaces = re.compile(r'[^\s]\s+$')
        self.pattern_spaces_start = re.compile(r'\t* +')
        self.pattern_tabs_start = re.compile(r'^\t+$')
        self.pattern_spaces_middle = re.compile(r'  +')
        self.pattern_space_operator = re.compile(r'\(([!]) ')
        self.pattern_comment_single = re.compile(r'//')
        self.pattern_tab_inside = re.compile(r'\S\t')
        self.pattern_carriage_return = re.compile(r'\r')
        self.pattern_comma = re.compile(r',[^ ]')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.carriage_return_count = 0

    def check(self, line_number, line):
        if self.pattern_whitespaces.search(line):
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Whitespace at line ending'))
        if self.pattern_spaces_start.match(line):
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Spaces at beginning of a line'))
        if self.pattern_tabs_start.match(line):
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Tabs in empty line'))

        operator_match = self.pattern_space_operator.search(line)
        if operator_match:
            error_msg = 'Space after operator >>{}<<'.format(operator_match.group(1))
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_msg))
        if self.pattern_tab_inside.search(line):
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Tab present inside the text'))

        if self.pattern_spaces_middle.search(line) or self.pattern_comma.search(line):
            temp = strip_comments_and_strings(line)
            if self.pattern_spaces_middle.search(temp):
                self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Spaces in the middle'))

            got_comma = self.pattern_comma.search(temp)
            if got_comma and got_comma.group(0) not in [',)']:
                self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Comma should be followed by a space'))

        if self.pattern_carriage_return.search(line):
            self.carriage_return_count += 1

    def finalize(self):
        if 0 < self.carriage_return_count:
            error_msg = 'Carriage returns present in file {} occurences'.format(self.carriage_return_count)
            self.error_reporter(self.NAME, Line(self.path, '', 0, error_msg))


class TestClassMacroValidator(SimpleValidator):
    SUITE_NAME = 'TestClassMacro'
    NAME = 'testClassMacro'

    def __init__(self):
        super().__init__()
        self.pattern_test_class = re.compile(r'#define [A-Z_]*TEST_CLASS (\s+)')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.line_test_class = None
        self.match_line_number = 0
        self.has_mismatched_test_class = False

        splitted = re.split(r'[/\\]', path)
        self.filename = splitted[-1]

    def match(self, line, line_number):
        has_match = self.pattern_test_class.search(line)
        if has_match:
            self.line_test_class = line
            self.match_line_number = line_number

            # some files have multiple test class macros, so just check that they end with the filename
            if not (has_match.group(1) + '.cpp').endswith(self.filename):
                self.has_mismatched_test_class = True

        return has_match

    def check(self, line_number, line):
        if self.match_line_number and self.match_line_number + 1 == line_number and line:
            if not self.match(line, line_number):
                self.error_reporter(self.NAME, Line(self.path, self.line_test_class, self.match_line_number))

        self.match(line, line_number)

    def finalize(self):
        if self.has_mismatched_test_class:
            self.error_reporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def format_error(err):
        name = err.path
        err_msg = '{}:{} TEST_CLASS should be followed by an empty line and match filename: >>{}<<'
        return err_msg.format(name, err.lineno, err.line)


class LineLengthValidator(SimpleValidator):
    SUITE_NAME = 'LongLines'
    NAME = 'tooLongLines'

    def __init__(self, line_length_limit=140):
        super().__init__()
        self.line_length_limit = line_length_limit

    def check(self, line_number, line):
        temp = re.sub('\t', '    ', line)
        if len(temp) >= self.line_length_limit:
            self.error_reporter(self.NAME, Line(self.path, line.strip(), line_number))

    @staticmethod
    def format_error(err):
        name = err.path
        return '{}:{} Line too long: >>{}<<'.format(name, err.lineno, err.line)


class TemplateSpaceValidator(SimpleValidator):
    SUITE_NAME = 'Template'
    NAME = 'templateFollowedBySpace'

    def __init__(self):
        super().__init__()
        self.pattern_template = re.compile(r'template\s+<')

    def check(self, line_number, line):
        if self.pattern_template.search(line):
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'Template followed by space'))


class CatchWithoutClosingTryBrace(SimpleValidator):
    SUITE_NAME = 'Catch Formatting'
    NAME = 'catchAndClosingTryBraceOnSeparateLines'

    def __init__(self):
        super().__init__()
        self.pattern_template = re.compile(r'^\s+catch')

    def check(self, line_number, line):
        if self.pattern_template.search(line):
            error_msg = 'catch and closing try brace must be on same line'
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_msg))


class SingleLineValidator(SimpleValidator):
    SUITE_NAME = 'SingleLine'
    NAME = 'singleLine'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.num_open = 0
        self.first_line = 0
        self.long = ''

    def check(self, line_number, line):
        if self.num_open:
            stripped = line.lstrip()
            if stripped.startswith('//'):
                self.num_open = 0
                return

            self.long = self.long + stripped
            if self.long.endswith(','):
                self.long += ' '

            temp = strip_comments_and_strings(stripped)

            num_brackets = 0
            had_brace = False
            for char in temp:
                if '(' == char:
                    self.num_open += 1
                elif ')' == char:
                    self.num_open -= 1
                elif char in '[]':
                    num_brackets += 1
                elif '{' == char:
                    had_brace = True

            if had_brace and num_brackets > 1:
                self.num_open = 0
                return

            if 0 >= self.num_open:
                temp = re.sub('\t', '    ', self.long)
                if len(temp) < 140:
                    error_msg = 'block fits in a single line'
                    self.error_reporter(self.NAME, Line(self.path, self.long, self.first_line, error_msg))

        # find lines ending with ( - possible function calls or definitions/declarations
        # but ignore raw string literals
        if line.endswith('(') and not line.endswith('R"('):
            self.num_open = 1
            self.first_line = line_number
            self.long = line


class PragmaOnceValidator(SimpleValidator):
    SUITE_NAME = 'Pragmas'
    NAME = 'pragmaErrors'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        # only .h files need pragma once
        self.got_pragma_once = None if path.endswith('.h') else True
        self.got_license = False
        self.empty_line_number = 0
        self.report_empty_line_error = None if path.endswith('.h') else False
        self.inside_comment = 0

    def check(self, line_number, line):
        # detect header notice and skip
        if line.startswith('/**'):
            self.inside_comment = 1
            self.got_license = True

        if 1 == self.inside_comment:
            if '**/' in line:
                self.inside_comment = 2
            return

        # used to skip empty line after closing comment
        if 2 == self.inside_comment:
            self.inside_comment = 3
            return

        if line.startswith('#include'):
            # we don't want empty line between pragma and include
            if self.report_empty_line_error is None:
                self.report_empty_line_error = 0 < self.empty_line_number

        if line.startswith('#'):
            if self.report_empty_line_error is None:
                self.report_empty_line_error = False

        if self.got_pragma_once:
            if not line:
                self.empty_line_number = line_number

        if self.got_pragma_once is None:
            self.got_pragma_once = line == '#pragma once'

    @staticmethod
    def format_error(err):
        name = err.path
        return '{} {}'.format(name, err.kind)

    def finalize(self):
        if not self.got_license:
            self.error_reporter(self.NAME, Line(self.path, '', 0, 'Missing license info'))

        if not self.got_pragma_once:
            self.error_reporter(self.NAME, Line(self.path, '', 0, 'Missing `#pragma once`'))

        if self.report_empty_line_error:
            self.error_reporter(self.NAME, Line(self.path, '', self.empty_line_number, 'Empty line after `#pragma once`'))


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
            re.compile(r'\)\{$'): 'missing space before brace',
            re.compile(r'boost/(filesystem|thread.hpp)'): 'use std',
            re.compile(r'boost::(filesystem|thread)'): 'use std',
            re.compile(r'#include <cstd'): 'use C header'
        }

    def check(self, line_number, line):
        for k, error_msg in self.errors.items():
            if k.search(line):
                self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_msg))


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

    def check(self, line_number, line):
        if re.match(r'src.catapult.functions.h', self.path):
            return

        for k, error_msg in self.errors.items():
            if k.search(line):
                self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_msg))


class SpaceBraceValidator(SimpleValidator):
    """Validates there is no space before brace, if intended usage is variable or object uniform initialization"""

    SUITE_NAME = 'SpaceBrace'
    NAME = 'spaceBrace'

    def __init__(self):
        super().__init__()
        self.pattern_name_space_brace = re.compile(r'([a-zA-Z][a-zA-Z0-9:<>_]+) +{')
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

    def check(self, line_number, line):
        # we want to match things like Foo {
        # and things like std::set<Foo> {
        result = self.pattern_name_space_brace.search(line)
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

        self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, matched))

    @staticmethod
    def format_error(err):
        name = err.path
        return '{}:{} Space between type or variable >>{}<< and brace: >>{}<<'.format(name, err.lineno, err.kind, err.line)


class ReturnOnNewLineValidator(SimpleValidator):
    """Validates that return is placed in a separate line"""

    SUITE_NAME = 'ReturnOnNewLine'
    NAME = 'returnOnNewLine'

    def __init__(self):
        super().__init__()
        self.pattern_return = re.compile(r'\S\s*return .*;')
        self.skip = [
            # skip if it looks like lambda
            re.compile(r']\(.*\)( mutable)? -> [^\r\n\t\f]+ {'),
            re.compile(r'\[.*\]\(.*\)( mutable)? { return'),

            re.compile(r'static constexpr .* return'),

            # skip comments
            re.compile(r'^\s*//')
        ]

    def check(self, line_number, line):
        result = self.pattern_return.search(line)
        if not result:
            return

        for pattern in self.skip:
            if pattern.search(line):
                return

        self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number))

    @staticmethod
    def format_error(err):
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
        # pylint: disable=invalid-name

        super().__init__()
        self.pattern_operator_bool = re.compile(r'[^\:]operator bool')
        self.pattern_operator_bool_with_explicit = re.compile(r'explicit operator bool')
        self.pattern_test = re.compile(r'\s+(NO_STRESS_)?TEST\(')
        self.pattern_test_class = re.compile(r'TEST\([A-Z_]*TEST_CLASS')
        self.pattern_validation_result = re.compile(r',\s+ValidationResult\s+(\w+)\)')
        self.is_test_validator = False
        self.pattern_missing_explicit_ctor = re.compile(r'^\s+(\w+)\((?:const )?(\w+)[&]? \w+\);$')
        self.pattern_enum = re.compile(r'enum [^:]')
        self.pattern_enum_class = re.compile(r'enum class ')
        self.pattern_coerce = re.compile(r'\s+(.*)auto(.).*= (ionet::)?CoercePacket')
        self.pattern_define_tests = re.compile(r'#define [A-Z_]*DEFINE_[A-Z_]*_TEST[^S]')
        self.pattern_define_test_traits = re.compile(r'#define [A-Z_]*DEFINE_[A-Z_]*_TEST_TRAITS[A-Z_]*')
        self.pattern_file_size = re.compile(r'FileSize&')
        self.pattern_file_size_reference_allowed = re.compile(r'operator|TryParseValue|cacheSize')
        self.pattern_file_size_cast = re.compile(r'const_cast<utils::FileSize&>.* = ')
        self.pattern_test_expected_size = re.compile(r'expected\w*Size =[^=]')

        # never allow memcmp in an assert
        self.pattern_test_memcmp_assert = re.compile(r'(ASSERT|EXPECT).*memcmp')

        # disallow == and != in an assert unless (1) iterator check with `ASSERT_TRUE(c.end() != iter)` or (2) explicit operators
        self.pattern_test_bool_assert = re.compile(r'(ASSERT|EXPECT)_(TRUE|FALSE).*(==|!=)')
        self.pattern_test_bool_assert_allowed = re.compile(r'(ASSERT_TRUE.*end\(\) !=)|operator(==|!=)')

        self.pattern_declare_macro_no_params = re.compile(r'DECLARE_(.+_)?(OBSERVER|VALIDATOR).*\(\)')

        self.pattern_single_line_function = re.compile(r'^[^\[\]]*\) { .*; }')
        self.pattern_test_single_line_function = re.compile(r'TEST_CLASS|TEST_NAME')

        self.pattern_test_name_if = re.compile(r'TEST.*If|\b(Next|Remove)When')
        self.pattern_test_name_if_exclusions = re.compile(r'\b(Next|Remove)If')

        self.pattern_header_comment = re.compile(r'^[^/]*// .*\.$')
        self.pattern_doxygen_comment = re.compile(r'///')

        self.pattern_auto_context_param = re.compile(r'auto& (context|notification)')

        self.pattern_gets_sets_doc = re.compile(r'/// (Gets|Sets) ')
        self.pattern_gets_sets_doc_with_article = re.compile(r'/// (Gets|Sets) (a|an|the|all|information)\b')

        self.pattern_trailing_operator = re.compile(r' (\+|-|\*|/|%|&|\||^|<<|>>)\s*$')

        self.pattern_struct_assignment = re.compile(r'^\t*(static )?(const(expr)? )?([a-zA-Z][a-zA-Z0-9:_]+) [a-zA-Z_][a-zA-Z0-9_]+ = {')

        self.errors = {
            self.check_test_line: 'TEST should use TEST_CLASS',
            self.check_explicit_operator_bool: 'Missing explicit before operator bool',
            self.check_validation_result: 'ValidationResult should not be last argument or should be called `value`',
            self.check_explicit_ctor: 'missing explicit before ctor',
            self.check_enum_class: 'use enum class instead of enum',
            self.check_coerce: 'use const auto* .. = CoercePacket',
            self.check_define_tests: 'use MAKE_ for singular TEST',
            self.check_file_size: 'FileSize should be passed by value',
            self.check_test_expected_size: 'first size part should start on own line',
            self.check_test_asserts: 'use a different EXPECT or ASSERT macro',
            self.check_declare_macro_no_params: 'use DEFINE macro',
            self.check_single_line_function: 'reformat info multiple lines',
            self.check_test_name_if: 'use When instead of If',
            self.check_header_comment: '. unexpected in header file comment',
            self.check_cpp_doxygen_comment: '/// unexpected in cpp file',
            self.check_auto_context_param: 'use type name instead of auto',
            self.check_gets_sets_documentation: 'add an article to documentation',
            self.check_trailing_operator: 'operators should start lines, not finish them',
            self.check_struct_assignment: 'prefer struct initialization to struct assignment'
        }

    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        # match common part of validation, validator
        self.is_test_validator = re.search(r'validat', path, re.IGNORECASE) and re.search(r'test', path, re.IGNORECASE)

    def check_explicit_operator_bool(self, line, _):
        return self.pattern_operator_bool.search(line) and not self.pattern_operator_bool_with_explicit.search(line)

    def check_test_line(self, line, _):
        # special file, skip it
        if self.path.endswith('Stress.h'):
            return False

        # if doesn't contain TEST( it's not interesting at all
        if not self.pattern_test.search(line):
            return False

        # if contains TEST(TEST_CLASS, mark as ok
        if self.pattern_test_class.search(line):
            return False

        # macros redefining TEST should use TEST_NAME
        if re.search(r'TEST_NAME', line):
            return False

        # if it contains preprocessor concatenation, it means it's probably inside some other #define
        if re.search(r'##', line):
            return False

        # mark as invalid
        return True

    def check_explicit_ctor(self, line, _):
        match = self.pattern_missing_explicit_ctor.match(line)
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

    def check_validation_result(self, line, _):
        if not self.is_test_validator:
            return False

        match = self.pattern_validation_result.search(line)
        if match:
            return match.group(1) != 'value'

        return False

    def check_enum_class(self, line, _):
        return self.pattern_enum.search(line) and not self.pattern_enum_class.search(line)

    def check_coerce(self, line, _):
        match = self.pattern_coerce.search(line)
        if match:
            return match.group(1) != 'const ' or match.group(2) != '*'

        return False

    def check_define_tests(self, line, _):
        return self.pattern_define_tests.match(line) and not self.pattern_define_test_traits.match(line)

    def check_file_size(self, line, _):
        if not self.pattern_file_size.search(line):
            return False

        # ignore lines where FileSize reference is explicitly allowed
        if self.pattern_file_size_reference_allowed.search(line):
            return False

        # ignore cast
        if self.pattern_file_size_cast.search(line):
            return False

        return True

    def check_test_expected_size(self, line, _):
        return self.pattern_test_expected_size.search(line) and ';' not in line

    def check_test_asserts(self, line, _):
        # special file, skip it
        if self.path.endswith('TestHarness.h'):
            return False

        # fail if memcmp is used in a test assert
        if self.pattern_test_memcmp_assert.search(line):
            return True

        # fail if == or != are used in a test assert
        return self.pattern_test_bool_assert.search(line) and not self.pattern_test_bool_assert_allowed.search(line)

    def check_declare_macro_no_params(self, line, _):
        # rule only applies to cpp files
        return self.path.endswith('.cpp') and self.pattern_declare_macro_no_params.search(line)

    def check_single_line_function(self, line, _):
        return self.pattern_single_line_function.search(line) and not self.pattern_test_single_line_function.search(line)

    def check_test_name_if(self, line, _):
        return self.pattern_test_name_if.search(line) and not self.pattern_test_name_if_exclusions.search(line)

    def check_header_comment(self, _, raw_line):
        # rule only applies to header files
        return self.path.endswith('.h') and self.pattern_header_comment.match(raw_line)

    def check_cpp_doxygen_comment(self, _, raw_line):
        # rule only applies to cpp files
        return self.path.endswith('.cpp') and self.pattern_doxygen_comment.search(raw_line)

    def check_auto_context_param(self, line, _):
        # rule only applies to observer and validator implementations
        return self.path.endswith(('Observer.cpp', 'Validator.cpp')) and self.pattern_auto_context_param.search(line)

    def check_gets_sets_documentation(self, _, raw_line):
        return self.pattern_gets_sets_doc.search(raw_line) and not self.pattern_gets_sets_doc_with_article.search(raw_line)

    def check_trailing_operator(self, line, _):
        # not part of SimpleValidator because comments and strings should be removed before applying rule
        return self.pattern_trailing_operator.search(line)

    def check_struct_assignment(self, line, _):
        match = self.pattern_struct_assignment.match(line)
        if match:
            # treat auto assignments as valid ones (`auto foo = { ... }`)
            return 'auto' != match.group(4)

        return False

    def check(self, line_number, line):
        stripped_line = strip_comments_and_strings(line)
        for func, error_msg in self.errors.items():
            if func(stripped_line, line):
                self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_msg))


class Cpp17TraitsValidator(SimpleValidator):
    """Validates that cpp17 traits are used in most files"""

    SUITE_NAME = 'Cpp17Traits'
    NAME = 'cpp17Traits'

    # pylint: disable=attribute-defined-outside-init
    def __init__(self):
        super().__init__()
        self.type_pattern = re.compile(r'>::type\b')
        self.value_pattern = re.compile(r'>::value\b')

    def check(self, line_number, line):
        #  skip custom traits files that need to use ::value and ::type
        if self.path.endswith(('Traits.h', 'StlTraits.h', 'TraitsTests.cpp', 'StlTraitsTests.cpp')):
            return

        error_message = ''
        if self.value_pattern.search(line):
            error_message = 'use _v instead of ::value'

        #  boost::logging requires one usage of ::type
        if not self.path.endswith('Logging.h') and self.type_pattern.search(line):
            error_message = 'use _t instead of ::type'

        if not error_message:
            return

        self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, error_message))


def rindex(lst, searched):
    return next(idx for idx, value in zip(range(len(lst)-1, -1, -1), reversed(lst)) if value == searched)


class UtilsSubdirValidator(SimpleValidator):
    """Validates there is no `utils` subdir under tests"""

    SUITE_NAME = 'UtilsSubdir'
    NAME = 'utilsSubdir'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.has_utils = False
        splitted = re.split(r'[/\\]', path)
        if 'tests' in splitted and 'utils' in splitted:
            idx = rindex(splitted, 'utils')
            # skip catapult/utils
            if idx > 0 and splitted[idx - 1] != 'catapult':
                self.has_utils = True

    def check(self, line_number, line):
        pass

    def finalize(self):
        if self.has_utils:
            self.error_reporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def format_error(err):
        name = err.path
        return '{} utils subdirectory should be replaced with test >>{}<<'.format(name, err.line)


class StressTestNameValidator(SimpleValidator):
    """Validates that stress tests have approprate names"""

    SUITE_NAME = 'StressTestName'
    NAME = 'stressTestName'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.has_improper_name = False
        splitted = re.split(r'[/\\]', path)
        if 'tests' in splitted:
            # skip validation of test util names
            if 'stress' in splitted and 'test' not in splitted:
                if not splitted[-1].endswith('IntegrityTests.cpp'):
                    self.has_improper_name = True
            else:
                if splitted[-1].endswith('IntegrityTests.cpp'):
                    self.has_improper_name = True

    def check(self, line_number, line):
        pass

    def finalize(self):
        if self.has_improper_name:
            self.error_reporter(self.NAME, Line(self.path, '', 0))

    @staticmethod
    def format_error(err):
        name = err.path
        return '{} all stress tests and only stress tests should end with `IntegrityTests.cpp`>>{}<<'.format(name, err.line)


class RegionValidator(SimpleValidator):
    """Validates nested regions and region typos"""

    SUITE_NAME = 'RegionValidator'
    NAME = 'regionValidator'

    def __init__(self):
        super().__init__()
        self.pattern_region = re.compile(r'//(.*)region')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.counter = 0
        self.previous_region_line = None
        self.previous_region_line_number = 0
        self.first_before_nested_line = None
        self.first_before_nested_line_number = 0
        self.errors = []

    def check(self, line_number, line):
        result = self.pattern_region.search(line)
        if not result:
            return

        prefix = result.group(1)
        if prefix == ' ':
            if self.counter > 0:
                if not self.first_before_nested_line:
                    self.first_before_nested_line = self.previous_region_line
                    self.first_before_nested_line_number = self.previous_region_line_number
                msg = 'nested region (top-most in line:{})'.format(self.first_before_nested_line_number)
                self.errors.append(Line(self.path, line.strip('\n\r'), line_number, msg))

            self.previous_region_line = line.strip('\n\r')
            self.previous_region_line_number = line_number
            self.counter += 1
        elif prefix == ' end':
            if self.counter == 0:
                self.errors.append(Line(self.path, line.strip('\n\r'), line_number, 'endregion without corresponding region'))

            self.counter -= 1
        else:
            # if name is invalid report immediatelly
            self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'invalid region "{}"'.format(result.group(0))))

    def finalize(self):
        if self.counter > 0:
            line = self.first_before_nested_line or self.previous_region_line
            line_number = self.first_before_nested_line_number or self.previous_region_line_number
            self.error_reporter(self.NAME, Line(self.path, line, line_number, 'non-closed region (probable location)'))

        elif self.errors:
            for error in self.errors:
                self.error_reporter(self.NAME, error)


class MacroSemicolonValidator(SimpleValidator):
    """Validator for ensuring that macros don't have trailing semicolons."""

    SUITE_NAME = 'MacroSemicolonChecker'
    NAME = 'macroSemicolonChecker'

    def __init__(self):
        super().__init__()
        self.macro_call = re.compile(r'^\s+[A-Z_]+\([^\)]*\);')

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

    def check(self, line_number, line):
        stripped_line = strip_comments_and_strings(line)

        if not self.macro_call.match(stripped_line):
            return

        for pattern in self.skip:
            if pattern.search(stripped_line):
                return

        self.error_reporter(self.NAME, Line(self.path, line.strip('\n\r'), line_number, 'macro should not have trailing semicolon'))


class EnumValueBlankLineValidator(SimpleValidator):
    """Validator for ensuring that ENUM_VALUE entries are followed by blank lines."""

    SUITE_NAME = 'EnumValueBlankLineChecker'
    NAME = 'enumValueBlankLineChecker'

    def __init__(self):
        super().__init__()
        self.pattern_enum_value = re.compile(r'\s*ENUM_VALUE\(.*\) \\')
        self.pattern_blank_line = re.compile(r'\s*\\')
        self.previous_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r')

        if self.pattern_enum_value.match(self.previous_line) and not self.pattern_blank_line.match(stripped_line):
            self.error_reporter(
                self.NAME,
                Line(self.path, self.previous_line, line_number - 1, 'enum value is missing following blank line'))

        self.previous_line = stripped_line


class TrailingCommaValidator(SimpleValidator):
    """Validator for ensuring that there are no trailing commas."""

    SUITE_NAME = 'TrailingCommaChecker'
    NAME = 'trailingCommaChecker'

    def __init__(self):
        super().__init__()
        self.previous_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t')  # strip tabs too

        if self.previous_line and ',' == self.previous_line[-1]:
            if stripped_line and stripped_line[0] in [')', ']', '}']:
                self.error_reporter(self.NAME, Line(self.path, self.previous_line, line_number - 1, 'delete trailing comma'))

        self.previous_line = stripped_line


class ClosingBraceVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring that closing braces are properly followed by whitespace."""

    SUITE_NAME = 'ClosingBraceVerticalSpacingChecker'
    NAME = 'closingBraceVerticalSpacingVChecker'

    def __init__(self):
        super().__init__()
        self.pattern_closing_brace = re.compile(r'^\s*}\s*$')
        self.pattern_inbetween_closing_brace = re.compile(r'^\s*}[}\)]*;?\s*$')
        self.pattern_line_after_closing_brace = \
            re.compile(r'^#|^\s*(}[}\)]*[;,]?|namespace .*|break;|} catch.*|} else.*|])$')  # pylint: disable=invalid-name
        self.recent_lines = None

    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.recent_lines = ['', '', '']

    def check(self, line_number, line):
        self.recent_lines.append(line.strip('\n\r'))
        self.recent_lines.pop(0)

        # check if line following brace is valid:
        # 1. preprocessor directive
        # 2. another closing brace (with optional closing symbols and/or semicolon and/or comma)
        # 3. namespace statement within forward declarations
        # 4. break statement within switch
        # 5. catch statement
        # 6. closing square bracket (inline JSON)
        if self.pattern_closing_brace.match(self.recent_lines[1]):
            if self.recent_lines[2] and not self.pattern_line_after_closing_brace.match(self.recent_lines[2]):
                self.error_reporter(self.NAME, Line(self.path, self.recent_lines[2], line_number, 'improper line following closing brace'))

        # check if line between closing braces is valid:
        # 1. blank
        # 2. valid line following brace
        if self.pattern_closing_brace.match(self.recent_lines[2]) and self.pattern_closing_brace.match(self.recent_lines[0]):
            if not self.recent_lines[1] or not self.pattern_line_after_closing_brace.match(self.recent_lines[1]):
                self.error_reporter(
                    self.NAME,
                    Line(self.path, self.recent_lines[1], line_number - 1, 'improper line between closing braces'))


class NamespaceOpeningBraceVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring that opening namespaces are properly followed by whitespace or not."""

    SUITE_NAME = 'NamespaceOpeningBraceVerticalSpacingChecker'
    NAME = 'namespaceOpeningBraceVerticalSpacingChecker'

    def __init__(self):
        super().__init__()
        self.pattern_namespace_opening = re.compile(r'^(\s*(inline )?namespace \w+ {)+$')
        self.pattern_anon_namespace_opening = re.compile(r'^\s*namespace {')
        self.pattern_forward_declaration = re.compile(r'^\s*(namespace \w+ { )*(class|struct) \w+;')
        self.recent_lines = None

    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.recent_lines = ['', '', '']

    def check(self, line_number, line):
        self.recent_lines.append(line.strip('\n\r'))
        self.recent_lines.pop(0)

        is_outer_namespace = False
        is_inner_or_anon_namespace = False
        if self.pattern_namespace_opening.match(self.recent_lines[1]):
            if '\t' != self.recent_lines[1][0]:
                is_outer_namespace = True
            else:
                is_inner_or_anon_namespace = True
        elif self.pattern_anon_namespace_opening.match(self.recent_lines[1]):
            is_inner_or_anon_namespace = True

        # check line following outer namespace opening
        following_line = self.recent_lines[2]
        if is_outer_namespace and following_line:
            if not (self.pattern_namespace_opening.match(following_line) or self.pattern_forward_declaration.match(following_line)):
                self.report_error(line_number, 'line following namespace opening must be blank or namespace opening or forward declaration')

        # check line following inner or anon namespace opening
        if is_inner_or_anon_namespace and not following_line:
            self.report_error(line_number, 'line following anon or inner namespace opening must not be blank')

    def report_error(self, line_number, message):
        self.error_reporter(self.NAME, Line(self.path, self.recent_lines[1], line_number - 1, message))


class CopyrightCommentValidator(SimpleValidator):
    """Validator for ensuring copyright comment consistency."""

    SUITE_NAME = 'CopyrightCommentChecker'
    NAME = 'copyrightCommentChecker'

    def __init__(self):
        super().__init__()
        self.expected_hash = unhexlify('261ac1f521fdc6a8de61d71a65e105cea0838713')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path, error_reporter):
        super().reset(path, error_reporter)
        self.hasher = sha1()
        self.last_line_number = 0

    def check(self, line_number, line):
        self.last_line_number = line_number
        if line_number <= 20:
            self.hasher.update(bytes(line, 'utf-8'))

        if line_number == 20:
            if self.expected_hash != self.hasher.digest():
                self.error_reporter(self.NAME, Line(self.path, '', 1))

    def finalize(self):
        if 20 > self.last_line_number:
            self.error_reporter(self.NAME, Line(self.path, '', 1))

    @staticmethod
    def format_error(err):
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
        self.is_opening_brace_unclosed = False
        self.previous_stripped_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t')  # also strip tabs

        # `{}` must be on its own line expect in (2) cases
        if stripped_line.endswith(r'{}') and stripped_line != r'{}':
            if not any(stripped_line.startswith(prefix) for prefix in ['virtual ~', 'while ']):
                self.report_error(line_number, line, 'empty statement body must be on new line')

        # `{};` must never be on its own line unless preceeded by what looks like class continuation
        if stripped_line.endswith(r'{};'):
            if any(self.previous_stripped_line.startswith(prefix) or stripped_line.startswith(prefix) for prefix in [':', ',']):
                if stripped_line != r'{};':
                    self.report_error(line_number, line, 'empty statement body must be on new line ')
            else:
                if stripped_line == r'{};':
                    self.report_error(line_number, line, 'empty statement body must be part of previous line ')

        self.previous_stripped_line = stripped_line

        # mark opening statements
        if stripped_line.endswith(r'{'):
            self.is_opening_brace_unclosed = True
            return

        if not self.is_opening_brace_unclosed:
            return

        # empty statements are never allowed to be multiline
        self.is_opening_brace_unclosed = False
        if stripped_line == r'};':
            self.report_error(line_number, line, 'empty statement body must be part of previous line')

        if stripped_line == r'}':
            self.report_error(line_number, line, 'empty statement body must be on new line')

    def report_error(self, line_number, line, message):
        self.error_reporter(self.NAME, Line(self.path, line, line_number, message))


class DocumentationVerticalSpacingValidator(SimpleValidator):
    """Validator for ensuring documentation has appropriate vertical spacing."""

    SUITE_NAME = 'DocumentationVerticalSpacingChecker'
    NAME = 'documentationVerticalSpacingChecker'

    def __init__(self):
        super().__init__()
        self.previous_stripped_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t')  # also strip tabs

        if stripped_line.startswith('///'):
            if (self.previous_stripped_line
                    and all(not self.previous_stripped_line.startswith(postfix) for postfix in ['///', '#'])
                    and all(not self.previous_stripped_line.endswith(postfix) for postfix in [':', '{'])):
                self.report_error(line_number, line, 'documentation has unexpected previous line')

        self.previous_stripped_line = stripped_line

    def report_error(self, line_number, line, message):
        self.error_reporter(self.NAME, Line(self.path, line, line_number, message))


class InsertionOperatorFormattingValidator(SimpleValidator):
    """Validator for ensuring insertion operator has appropriate formatting."""

    SUITE_NAME = 'InsertionOperatorFormattingChecker'
    NAME = 'insertionOperatorFormattingChecker'

    def __init__(self):
        super().__init__()
        self.previous_stripped_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t;')  # also strip tabs and semicolons

        if stripped_line.startswith('<<'):
            if (any(stripped_line.endswith(postfix) for postfix in ['open_array', 'open_document'])
                    and self.previous_stripped_line.endswith('"')):
                self.report_error(line_number, stripped_line, '<< should be on previous line')

            if self.previous_stripped_line.startswith('<<'):
                return

            if '<<' in self.previous_stripped_line:
                self.report_error(line_number - 1, self.previous_stripped_line, '<< should be on own line')

        self.previous_stripped_line = stripped_line

    def report_error(self, line_number, line, message):
        self.error_reporter(self.NAME, Line(self.path, line, line_number, message))


class TestFirstLineCommentValidator(SimpleValidator):
    """Validator for ensuring that first comments of test functions are well formed."""

    SUITE_NAME = 'TestFirstLineCommentChecker'
    NAME = 'testFirstLineCommentChecker'

    def __init__(self):
        super().__init__()
        self.pattern_test_function = re.compile(r'TEST\(')
        self.pattern_ends_assert_comment = re.compile(r'//( -|.*Assert:$)')
        self.previous_stripped_line = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t')  # also strip tabs

        if self.pattern_test_function.search(self.previous_stripped_line):
            if self.pattern_ends_assert_comment.match(stripped_line):
                self.report_error(line_number, line, 'should remove redundant assert comment')

        self.previous_stripped_line = stripped_line

    def report_error(self, line_number, line, message):
        self.error_reporter(self.NAME, Line(self.path, line, line_number, message))


class ExplicitCtorValidator(SimpleValidator):
    """Validator for ensuring that only single argument constructors have explicit keyword."""

    SUITE_NAME = 'ExplicitCtorChecker'
    NAME = 'explicitCtorChecker'

    def __init__(self):
        super().__init__()
        self.pattern_explicit_ctor = re.compile(r'(constexpr )?explicit \w+\(')

        self.ctor_line_number = 0
        self.ctor_start_line = ''

        self.open_paren_count = 0
        self.open_angle_bracket_count = 0
        self.ctor_arguments = ''

    def check(self, line_number, line):
        stripped_line = line.strip('\n\r\t')  # also strip tabs

        if 0 == self.open_paren_count and not self.pattern_explicit_ctor.match(stripped_line):
            return

        if 0 == self.open_paren_count:
            # save for error reporting
            self.ctor_line_number = line_number
            self.ctor_start_line = stripped_line

        for char in stripped_line:
            if '<' == char:
                self.open_angle_bracket_count += 1
            elif '>' == char:
                self.open_angle_bracket_count -= 1
            if '(' == char:
                self.open_paren_count += 1
            elif ')' == char:
                self.open_paren_count -= 1
                if 0 == self.open_paren_count:
                    self.check_explicit_ctor_line()
                    self.ctor_arguments = ''
                    break
            else:
                if 0 == self.open_angle_bracket_count and 0 != self.open_paren_count:
                    self.ctor_arguments += char

    def check_explicit_ctor_line(self):
        if not self.ctor_arguments:
            self.report_error('empty ctor should not use explicit')

        # single arg ctor can always be explicit
        split_args = self.ctor_arguments.split(',')
        if 1 == len(split_args):
            return

        # if second arg has default, explicit is allowed
        if any(default_marker in split_args[1] for default_marker in ['=', '...']):
            return

        self.report_error('multi arg ctor should not use explicit')

    def report_error(self, message):
        self.error_reporter(self.NAME, Line(self.path, self.ctor_start_line, self.ctor_line_number, message))


def create_validators():
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
