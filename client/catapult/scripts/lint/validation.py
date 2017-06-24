import re

from SimpleValidator import SimpleValidator, Line
from forwardsValidation import ForwardsValidator

class WhitespaceLineValidator(SimpleValidator):
    SUITE_NAME = 'Whitespaces'
    NAME = 'whitespaceLines'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternWhitespaces = re.compile(r'[^\s]\s+$')
        self.patternSpacesStart = re.compile(r'\t* +')
        self.patternSpacesMiddle = re.compile(r'  +')
        self.patternSpaceOperator = re.compile(r'\(([!]) ')
        self.patternCommentSingle = re.compile(r'//')
        self.patternTabInside = re.compile(r'\S\t')
        self.patternCarriageReturn = re.compile(r'\r')

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        self.carriageReturnCount = 0

    def check(self, lineNumber, line):
        if re.search(self.patternWhitespaces, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Whitespace at line ending'))
        if re.match(self.patternSpacesStart, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces at beginning of a line'))
        operatorMatch = re.search(self.patternSpaceOperator, line)
        if operatorMatch:
            errorMsg = 'Space after operator >>{}<<'.format(operatorMatch.group(1))
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))
        if re.search(self.patternTabInside, line):
            self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Tab present inside the text'))

        if re.search(self.patternSpacesMiddle, line):
            # drop inline comments, remove strings and comments
            temp = re.sub(r'//.*', '', line)
            temp = re.sub(r'"(.+?)"', 'dummy', temp)
            temp = re.sub(r'/\*(.+?)\*/', 'dummy', temp)
            if re.search(self.patternSpacesMiddle, temp):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, 'Spaces in the middle'))

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

class PragmaOnceValidator(SimpleValidator):
    SUITE_NAME = 'Pragmas'
    NAME = 'pragmaErrors'

    # pylint: disable=attribute-defined-outside-init
    def reset(self, path):
        super().reset(path)
        # only .h files need pragma once
        self.gotPragmaOnce = None if path.endswith('.h') else True
        self.emptyLineNumber = 0
        self.reportEmptyLineError = None if path.endswith('.h') else False

    def check(self, lineNumber, line):
        if line.startswith('#include'):
            # we don't want empty line between pragma and include
            if self.reportEmptyLineError is None:
                self.reportEmptyLineError = 0 < self.emptyLineNumber

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
            re.compile(r'{ +}'): 'don\'t leave space between braces `{}`',
            re.compile(r'///.*triggered by .* notification[^s]'): 'notification*S* DUMBASS'
        }

    def check(self, lineNumber, line):
        for k, errorMsg in self.errors.items():
            if re.search(k, line):
                self.errorReporter(self.NAME, Line(self.path, line.strip('\n\r'), lineNumber, errorMsg))

    @staticmethod
    def formatError(err):
        name = err.path
        return '{}:{} TYPO {}: >>{}<<'.format(name, err.lineno, err.kind, err.line)

class SpaceBraceValidator(SimpleValidator):
    SUITE_NAME = 'SpaceBrace'
    NAME = 'spaceBrace'

    def __init__(self, errorReporter):
        super().__init__(errorReporter)
        self.patternNameSpaceBrace = re.compile(r'([a-zA-Z][a-zA-Z0-9:<>_]+) +{')
        self.skip = [
            # skip if line contains 'namespace', that line shouldn't be interesting
            re.compile(r'namespace'),

            # skip if it looks like method/function/lambda definition
            re.compile(r'(const|override|noexcept|mutable) {'),

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

def createValidators(errorReporter):
    validators = [
        WhitespaceLineValidator,
        LineLengthValidator,
        TemplateSpaceValidator,
        CatchWithoutClosingTryBrace,
        PragmaOnceValidator,
        TypoChecker,
        SpaceBraceValidator,
        ForwardsValidator
    ]
    return list(map(lambda validator: validator(errorReporter), validators))
