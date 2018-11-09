# pylint: disable=too-few-public-methods
import re
from .CatsParseException import CatsParseException
from .CompositeTypeParser import CompositeTypeParser
from .RegexParserFactory import RegexParserFactory
from .parserutils import is_builtin, is_dec_or_hex, is_uint, parse_builtin, parse_dec_or_hex, require_property_name, require_user_type_name


class StructParser(CompositeTypeParser):
    """Parser for `struct` statements"""
    def __init__(self, regex):
        super().__init__(regex, [
            StructConstParserFactory(),
            StructInlineParserFactory(),
            StructMemberParserFactory()
        ])

    def process_line(self, line):
        match = self.regex.match(line)
        self.type_name = require_user_type_name(match.group(1))
        self.type_descriptor = {'type': 'struct', 'layout': []}

    def append(self, property_type_descriptor):
        if 'size' in property_type_descriptor:
            self._require_known_property(property_type_descriptor['size'])

        descriptor_uid = self._get_descriptor_uid(property_type_descriptor)
        if descriptor_uid[0]:
            self._require_unknown_property(descriptor_uid)

        self.type_descriptor['layout'].append(property_type_descriptor)

    def _require_known_property(self, property_name):
        # size can be a constant represented by a numeric type
        if not isinstance(property_name, str):
            return

        if all('name' not in property_type_descriptor or property_name != property_type_descriptor['name']
               for property_type_descriptor in self.type_descriptor['layout']):
            raise CatsParseException('no definition for referenced property "{0}"'.format(property_name))

    def _require_unknown_property(self, descriptor_uid):
        if any(descriptor_uid == self._get_descriptor_uid(property_type_descriptor)
               for property_type_descriptor in self.type_descriptor['layout']):
            raise CatsParseException('duplicate definition for property "{0}"'.format(descriptor_uid))

    @staticmethod
    def _get_descriptor_uid(descriptor):
        return (descriptor.get('name'), descriptor.get('disposition'))


class StructParserFactory(RegexParserFactory):
    """Factory for creating struct parsers"""
    def __init__(self):
        super().__init__(r'struct (\S+)', StructParser)


class StructConstParser:
    """Parser for const struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        type_name = match.group(1)

        const_descriptor = {
            'name': require_property_name(match.group(2)),
            'disposition': 'const',
            'value': parse_dec_or_hex(match.group(3))
        }

        if is_uint(type_name):
            const_descriptor = {**const_descriptor, **parse_builtin(type_name)}
        else:
            const_descriptor['type'] = require_user_type_name(type_name)

        return const_descriptor


class StructConstParserFactory(RegexParserFactory):
    """Factory for creating struct const parsers"""
    def __init__(self):
        super().__init__(r'const (\S+) (\S+) = (\S+)', StructConstParser)


class StructInlineParser:
    """Parser for inline struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        match = self.regex.match(line)
        return {'type': match.group(1), 'disposition': 'inline'}


class StructInlineParserFactory(RegexParserFactory):
    """Factory for creating struct inline parsers"""
    def __init__(self):
        super().__init__(r'inline (\S+)', StructInlineParser)


class StructMemberParser:
    """Parser for non-inline struct members"""
    def __init__(self, regex):
        self.regex = regex
        self.array_type_regex = re.compile(r'^array\((\S+), (\S+)(, sort_key=(\S+))?\)$')

    def process_line(self, line):
        match = self.regex.match(line)
        linked_type_name = match.group(2)
        array_type_match = self.array_type_regex.match(linked_type_name)

        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        if array_type_match:
            array_size = array_type_match.group(2)
            if is_dec_or_hex(array_size):
                array_size = parse_dec_or_hex(array_size)

            property_type_descriptor = {
                'type': array_type_match.group(1),
                'size': array_size
            }

            if array_type_match.group(3):
                property_type_descriptor['sort_key'] = array_type_match.group(4)

        else:
            if is_builtin(linked_type_name):
                property_type_descriptor = parse_builtin(linked_type_name)  # reduce builtins to byte
            else:
                property_type_descriptor = {'type': linked_type_name}

        property_type_descriptor['name'] = require_property_name(match.group(1))
        return property_type_descriptor


class StructMemberParserFactory(RegexParserFactory):
    """Factory for creating struct member parsers"""
    def __init__(self):
        super().__init__(r'(\S+) = ([\S,\(\) ]*\S)', StructMemberParser)
