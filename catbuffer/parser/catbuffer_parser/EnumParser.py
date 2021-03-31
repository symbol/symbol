from .CatsParseException import CatsParseException
from .CompositeTypeParser import CompositeTypeParser
from .parserutils import parse_builtin, parse_dec_or_hex, require_primitive, require_property_name, require_user_type_name
from .RegexParserFactory import RegexParserFactory


class EnumParser(CompositeTypeParser):
    """Parser for `enum` statements"""
    def __init__(self, regex):
        super().__init__(regex, [EnumValueParserFactory()])

    def process_line(self, line):
        match = self.regex.match(line)
        self.type_name = require_user_type_name(match.group(1))

        base_type = require_primitive(match.group(2))
        builtin_type_descriptor = parse_builtin(base_type)
        self.type_descriptor = {
            'type': 'enum',
            'size': builtin_type_descriptor['size'],
            'signedness': builtin_type_descriptor['signedness'],
            'values': []
        }

    def append(self, property_value_descriptor):
        self._require_unknown_property(property_value_descriptor['name'])

        self.type_descriptor['values'].append(property_value_descriptor)

    def _require_unknown_property(self, property_name):
        if any(property_name == property_type_descriptor['name'] for property_type_descriptor in self.type_descriptor['values']):
            raise CatsParseException('duplicate definition for enum value "{0}"'.format(property_name))


class EnumParserFactory(RegexParserFactory):
    """Factory for creating enum parsers"""
    def __init__(self):
        super().__init__(r'enum (\S+) : (u?int\d+)', EnumParser)


class EnumValueParser:
    """Parser for enum values"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        return {'name': require_property_name(match.group(1)), 'value': parse_dec_or_hex(match.group(2))}


class EnumValueParserFactory(RegexParserFactory):
    """Factory for creating enum value parsers"""
    def __init__(self):
        super().__init__(r'(\S+) = (\S+)', EnumValueParser)
