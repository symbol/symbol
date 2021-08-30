from .CatsParseException import CatsParseException
from .CompositeTypeParser import CompositeTypeParser
from .parserutils import (is_builtin, is_dec_or_hex, is_primitive, parse_builtin, parse_dec_or_hex, require_property_name,
                          require_user_type_name)
from .RegexParserFactory import RegexParserFactory

# region StructParser(Factory)


class StructParser(CompositeTypeParser):
    """Parser for `struct` statements"""
    def __init__(self, regex):
        super().__init__(regex, [
            StructConstParserFactory(),
            StructInlineParserFactory(),
            StructScalarMemberParserFactory(),
            StructArrayMemberParserFactory()
        ])

    def process_line(self, line):
        match = self.regex.match(line)
        self.type_name = require_user_type_name(match.group(1))
        self.type_descriptor = {'type': 'struct', 'layout': []}

    def append(self, property_type_descriptor):
        self._require_no_array_with_fill_disposition()

        if 'size' in property_type_descriptor:
            self._require_known_property(property_type_descriptor['size'])

        descriptor_uid = self._get_descriptor_uid(property_type_descriptor)
        if descriptor_uid[0]:
            self._require_unknown_property(descriptor_uid)

        self.type_descriptor['layout'].append(property_type_descriptor)

    def commit(self):
        for property_type_descriptor in self.type_descriptor['layout']:
            if 'condition' in property_type_descriptor:
                self._require_known_property(property_type_descriptor['condition'], False)

        return super().commit()

    def _require_no_array_with_fill_disposition(self):
        layout = self.type_descriptor['layout']
        if not layout:
            return

        last_property = layout[-1]
        if 'fill' == last_property.get('disposition'):
            raise CatsParseException('array property with fill disposition "{0}" must be last property'.format(last_property['name']))

    def _require_known_property(self, property_name, allow_numeric=True):
        # size can be a constant represented by a numeric type
        if allow_numeric and not isinstance(property_name, str):
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

# endregion

# region StructConstParser(Factory)


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

        if is_primitive(type_name):
            const_descriptor = {**const_descriptor, **parse_builtin(type_name)}
        else:
            const_descriptor['type'] = require_user_type_name(type_name)

        return const_descriptor


class StructConstParserFactory(RegexParserFactory):
    """Factory for creating struct const parsers"""
    def __init__(self):
        super().__init__(r'const (\S+) (\S+) = (\S+)', StructConstParser)


# endregion

# region StructInlineParser(Factory)

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

# endregion

# region StructScalarMemberParser(Factory)


class StructScalarMemberParser:
    """Parser for non-inline scalar struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        linked_type_name = match.group(2)

        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        if is_builtin(linked_type_name):
            property_type_descriptor = parse_builtin(linked_type_name)  # reduce builtins to byte
        else:
            property_type_descriptor = {'type': linked_type_name}

        if match.group(3):
            property_type_descriptor['condition'] = match.group(4)
            property_type_descriptor['condition_operation'] = match.group(5)
            property_type_descriptor['condition_value'] = match.group(6)

        property_type_descriptor['name'] = require_property_name(match.group(1))
        return property_type_descriptor


class StructScalarMemberParserFactory(RegexParserFactory):
    """Factory for creating struct scalar member parsers"""
    def __init__(self):
        super().__init__(r'(\S+) = (\S+)( if (\S+) (equals|has) (\S+))?', StructScalarMemberParser)

# endregion

# region StructArrayMemberParser(Factory)


class StructArrayMemberParser:
    """Parser for non-inline array struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)

        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        property_type_descriptor = {'type': match.group(2)}

        # size can be interpreted in different ways for count-based arrays
        # size must be a field reference for size-based arrays
        array_size = match.group(4)
        if not match.group(3):
            if is_dec_or_hex(array_size):
                array_size = parse_dec_or_hex(array_size)

            if '__FILL__' == array_size:
                array_size = 0
                property_type_descriptor['disposition'] = 'fill'
        else:
            property_type_descriptor['disposition'] = 'var'

        property_type_descriptor['size'] = array_size

        if match.group(5):
            property_type_descriptor['sort_key'] = match.group(6)

        property_type_descriptor['name'] = require_property_name(match.group(1))
        return property_type_descriptor


class StructArrayMemberParserFactory(RegexParserFactory):
    """Factory for creating struct array member parsers"""
    def __init__(self):
        super().__init__(r'(\S+) = array\((\S+), (size=)?(\S+)(, sort_key=(\S+))?\)', StructArrayMemberParser)

# endregion
