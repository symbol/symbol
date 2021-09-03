from .parserutils import TypeNameChecker, parse_builtin
from .RegexParserFactory import RegexParserFactory


class AliasParser:
    """Parser for `using` statements"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)

        # aliases are only supported for builtin types
        return (
            TypeNameChecker.require_user_type(match.group('alias_type_name')),
            parse_builtin(match.group('aliased_type_name'))
        )


class AliasParserFactory(RegexParserFactory):
    """Factory for creating alias parsers"""
    def __init__(self):
        super().__init__(r'using (?P<alias_type_name>\S+) = (?P<aliased_type_name>\S+)', AliasParser)
