# pylint: disable=too-few-public-methods

from enum import Enum
import os
import re
import shutil

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
        matchToType = {
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
        if word in matchToType:
            self.type = matchToType[word]
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


def fixTabs(line, count):
    if count == 0:
        return line

    # remove count tabs
    if count < 0:
        line = re.sub(r'\t', '', line, -count)
    else:
        line = '\t'*count + line
    return line


class HeaderParser:
    patternInclude = re.compile(r'\s*#\s*include[ \t]*(["<][^">]*[">])(.*)')
    patternPreprocessor = re.compile(r'\s*#\s*(\w*)')
    patternExtern = re.compile(r'\s*extern\s*')
    patternEmptyLine = re.compile(r'^\s*$')

    def __init__(self, errorReporter, path, simpleValidators, fixIndentsInFiles=False):
        self.errorReporter = errorReporter
        self.path = path
        self.lineNumber = 0
        self.preprocessor = []
        #
        self.includes = []
        self.includeError = None
        self.simpleValidators = simpleValidators

        self.fixes = []
        # need to open as binary so that python can see '\r\n'
        self.parseFile(open(self.path, 'rb'))
        if self.fixes:
            if fixIndentsInFiles:
                self.fixIndents(open(self.path, 'r'), open(self.path + '.tmp', 'wb'))
                os.remove(self.path)
                shutil.move(self.path + '.tmp', self.path)
            self.reportIndents()

    def fixIndents(self, inf, outf):
        fixNo = 0
        lineNumber = 1
        firstContinuation = False
        for line in inf:
            line = line.strip('\n')
            if fixNo < len(self.fixes) and lineNumber == self.fixes[fixNo].lineno:
                line = line.strip('\n')
                if MultilineMacro.PPLINE == self.fixes[fixNo].type:
                    line = line.strip()
                    firstContinuation = True
                else:
                    if firstContinuation:
                        tabsMatch = re.match(r'^\t+', line)
                        tabsCount = len(tabsMatch.group(0)) if tabsMatch else 0
                        firstContinuation = False
                    line = fixTabs(line, 1 - tabsCount)
                fixNo += 1
            line = line + '\n'
            lineNumber += 1
            outf.write(line.encode('utf-8'))

    def reportIndents(self):
        firstContinuation = False
        for fix in self.fixes:
            if MultilineMacro.PPLINE == fix.type:
                if re.match(r'\s+', fix.line):
                    errorMsg = 'preprocessor should be aligned to column 0'
                    self.errorReporter('indentedPreprocessor', Line(self.path, fix.line.strip('\n\r'), fix.lineno, errorMsg))
                    firstContinuation = True
            else:
                if firstContinuation:
                    tabsMatch = re.match(r'^\t+', fix.line)
                    tabsCount = len(tabsMatch.group(0)) if tabsMatch else 0
                    if 1 != tabsCount:
                        errorMsg = 'first continuation must have single indent'
                        self.errorReporter('indentedPreprocessor', Line(self.path, fix.line.strip('\n\r'), fix.lineno, errorMsg))
                    firstContinuation = False

    def processContinuation(self, line):
        self.fixes.append(IndentFix(MultilineMacro.CONTINUATION, self.lineNumber, line))
        return line and '\\' == line[-1]

    def processPreprocessor(self, line):
        multiline = False
        if '\\' == line[-1]:
            multiline = True
        # this is here to skip pragma from preprocessor indent check
        if line != '#pragma once':
            self.fixes.append(IndentFix(MultilineMacro.PPLINE, self.lineNumber, line))
        return multiline

    def parseFile(self, inputStream):
        self.lineNumber = 1
        pprev = None
        prev = None
        temp = None
        isEmptyLine = False
        prevEmptyLine = False
        multiline = False

        for validator in self.simpleValidators:
            validator.reset(self.path, self.errorReporter)

        for rawLine in inputStream:
            line = rawLine.decode('utf8')
            line = line.strip('\n')

            pprev = prev
            prev = temp
            temp = re.sub('\t', '    ', line)

            for validator in self.simpleValidators:
                validator.check(self.lineNumber, line)

            prevEmptyLine = isEmptyLine
            isEmptyLine = bool(self.patternEmptyLine.match(line))
            if isEmptyLine and prevEmptyLine:
                self.errorReporter('consecutiveEmpty', Line(self.path, pprev + prev + temp, self.lineNumber))

            if multiline:
                multiline = self.processContinuation(line)
            else:
                if self.patternInclude.match(line):
                    self.parseInclude(line)
                    self.fixes.append(IndentFix(MultilineMacro.PPLINE, self.lineNumber, line))
                elif self.patternPreprocessor.match(line):
                    self.parsePreprocessor(line)
                    multiline = self.processPreprocessor(line)
                elif self.patternExtern.match(line):
                    self.parseExtern(line)
            self.lineNumber += 1

        if prev and not prev.strip():
            self.errorReporter('emptyNearEnd', Line(self.path, pprev + prev + temp, self.lineNumber))

        for validator in self.simpleValidators:
            validator.finalize()

    def parseInclude(self, line):
        res = self.patternInclude.match(line)
        self.preprocessor.append(Include(line, self.lineNumber, res.group(1), res.group(2)))
        self.includes.append(res.group(1))

    def parsePreprocessor(self, line):
        res = self.patternPreprocessor.match(line)
        self.preprocessor.append(Preproc(line, self.lineNumber, res.group(1)))

    def parseExtern(self, line):
        self.preprocessor.append(Preproc(line, self.lineNumber, 'extern'))
