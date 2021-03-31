from .parserutils import parse_builtin, require_user_type_name
from .RegexParserFactory import RegexParserFactory


class AliasParser:
    """Parser for `using` statements"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)

        # aliases are only supported for builtin types
        return (
            require_user_type_name(match.group(1)),
            parse_builtin(match.group(2))
        )


class AliasParserFactory(RegexParserFactory):
    """Factory for creating alias parsers"""
    def __init__(self):
        super().__init__(r'using (\S+) = (\S+)', AliasParser)
