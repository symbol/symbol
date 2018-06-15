import re


class RegexParserFactory:
    """Base for top-level parser factories"""
    def __init__(self, regex, parser_type):
        self.regex = re.compile('^{0}$'.format(regex))
        self.parser_type = parser_type

    def is_match(self, line):
        """Returns True if the line is a match for this factory's parser"""
        return self.regex.match(line)

    def create(self):
        """Creates a new parser"""
        return self.parser_type(self.regex)
