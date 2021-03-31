from .RegexParserFactory import RegexParserFactory


class ImportResult:
    """Information about an import statement"""
    def __init__(self, import_file):
        self.import_file = import_file

    def __eq__(self, rhs):
        return isinstance(rhs, ImportResult) and self.import_file == rhs.import_file


class ImportParser:
    """Parser for `import` statements"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        return ImportResult(match.group(1))


class ImportParserFactory(RegexParserFactory):
    """Factory for creating import parsers"""
    def __init__(self):
        super().__init__(r'import "([\S ]+)"', ImportParser)
