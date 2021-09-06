from .CatsParseException import CatsParseException
from .CompositeTypeParser import CompositeTypeParser
from .parserutils import TypeNameChecker, is_builtin, is_dec_or_hex, is_primitive, parse_builtin, parse_dec_or_hex
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
        self.type_name = TypeNameChecker.require_user_type(match.group('struct_type_name'))
        self.type_descriptor = {'type': 'struct', 'layout': []}

        if match.group('inline'):
            self.type_descriptor['disposition'] = 'inline'

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
            raise CatsParseException('array property with fill disposition "{}" must be last property'.format(last_property['name']))

    def _require_known_property(self, property_name, allow_numeric=True):
        # size can be a constant represented by a numeric type
        if allow_numeric and not isinstance(property_name, str):
            return

        if all('name' not in property_type_descriptor or property_name != property_type_descriptor['name']
               for property_type_descriptor in self.type_descriptor['layout']):
            raise CatsParseException('no definition for referenced property "{}"'.format(property_name))

    def _require_unknown_property(self, descriptor_uid):
        if any(descriptor_uid == self._get_descriptor_uid(property_type_descriptor)
               for property_type_descriptor in self.type_descriptor['layout']):
            raise CatsParseException('duplicate definition for property "{}"'.format(descriptor_uid))

    @staticmethod
    def _get_descriptor_uid(descriptor):
        return (descriptor.get('name'), descriptor.get('disposition'))


class StructParserFactory(RegexParserFactory):
    """Factory for creating struct parsers"""
    def __init__(self):
        super().__init__(r'(?P<inline>inline )?struct (?P<struct_type_name>\S+)', StructParser)


# endregion

# region StructConstParser(Factory)

class StructConstParser:
    """Parser for const struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        type_name = match.group('type_name')

        disposition = match.group('disposition')
        property_name_checker = TypeNameChecker.require_const_property if 'const' == disposition else TypeNameChecker.require_property
        const_descriptor = {
            'name': property_name_checker(match.group('name')),
            'disposition': disposition,
            'value': match.group('value')
        }

        is_numeric = False
        if is_primitive(type_name):
            const_descriptor = {**const_descriptor, **parse_builtin(type_name)}
            is_numeric = True
        else:
            const_descriptor['type'] = TypeNameChecker.require_user_type(type_name)
            is_numeric = is_dec_or_hex(const_descriptor['value'])

        if is_numeric:
            const_descriptor['value'] = parse_dec_or_hex(const_descriptor['value'])

        return const_descriptor


class StructConstParserFactory(RegexParserFactory):
    """Factory for creating struct const parsers"""
    def __init__(self):
        super().__init__(r'(?P<name>\S+) = make_(?P<disposition>const|reserved)\((?P<type_name>\S+), (?P<value>\S+)\)', StructConstParser)


# endregion

# region StructInlineParser(Factory)

class StructInlineParser:
    """Parser for inline struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        match = self.regex.match(line)
        inline_descriptor = {'type': match.group('inline_type_name'), 'disposition': 'inline'}

        inline_name_prefix = match.group('name')
        if inline_name_prefix:
            inline_descriptor['name'] = inline_name_prefix

        return inline_descriptor


class StructInlineParserFactory(RegexParserFactory):
    """Factory for creating struct inline parsers"""
    def __init__(self):
        super().__init__(r'(?:(?P<name>\S+) = )?inline (?P<inline_type_name>\S+)', StructInlineParser)


# endregion

# region StructScalarMemberParser(Factory)

OPTIONAL_CONDITIONAL_REGEX_PATTERN = \
    r'(?: if (?P<condition_value>\S+) (?P<condition_negate>not )?(?P<condition_operation>equals|in) (?P<condition>\S+))?'


def parse_conditional_instruction(property_type_descriptor, match):
    if not match.group('condition_value'):
        return

    is_negated = 'not ' == match.group('condition_negate')

    property_type_descriptor['condition'] = match.group('condition')
    property_type_descriptor['condition_operation'] = ('not ' if is_negated else '') + match.group('condition_operation')

    condition_value = match.group('condition_value')
    if is_dec_or_hex(condition_value):
        condition_value = parse_dec_or_hex(condition_value)

    property_type_descriptor['condition_value'] = condition_value


class StructScalarMemberParser:
    """Parser for non-inline scalar struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)
        linked_type_name = match.group('type_name')

        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        if is_builtin(linked_type_name):
            property_type_descriptor = parse_builtin(linked_type_name)  # reduce builtins to byte
        else:
            property_type_descriptor = {'type': linked_type_name}

        parse_conditional_instruction(property_type_descriptor, match)

        property_type_descriptor['name'] = TypeNameChecker.require_property(match.group('name'))
        return property_type_descriptor


class StructScalarMemberParserFactory(RegexParserFactory):
    """Factory for creating struct scalar member parsers"""
    def __init__(self):
        super().__init__(
            r'(?P<name>\S+) = (?P<type_name>\S+)' + OPTIONAL_CONDITIONAL_REGEX_PATTERN,
            StructScalarMemberParser)


# endregion

# region StructArrayMemberParser(Factory)

class StructArrayMemberParser:
    """Parser for non-inline array struct members"""
    def __init__(self, regex):
        self.regex = regex

    def process_line(self, line):
        match = self.regex.match(line)

        # type is resolved to exist upstream, so its naming doesn't need to be checked here
        element_type_name = match.group('element_type_name')

        if 'byte' == element_type_name:
            raise CatsParseException('explicit use of byte is deprecated please use uint8 or int8 instead')

        property_type_descriptor = {}
        if is_primitive(element_type_name):
            builtin_type_descriptor = parse_builtin(element_type_name)
            element_type_name = builtin_type_descriptor['type']

            property_type_descriptor['element_disposition'] = {**builtin_type_descriptor}
            del property_type_descriptor['element_disposition']['type']

        property_type_descriptor['type'] = element_type_name

        # size can be interpreted in different ways for count-based arrays
        # size must be a field reference for size-based arrays
        array_size = match.group('size')
        if not match.group('byte_size'):
            if is_dec_or_hex(array_size):
                array_size = parse_dec_or_hex(array_size)

            if '__FILL__' == array_size:
                array_size = 0
                property_type_descriptor['disposition'] = 'array fill'  # expands to fill the rest of the structure
            else:
                property_type_descriptor['disposition'] = 'array'
        else:
            property_type_descriptor['disposition'] = 'array sized'  # fits a predefined size

        property_type_descriptor['size'] = array_size

        if match.group('sort_key'):
            property_type_descriptor['sort_key'] = match.group('sort_key')

        parse_conditional_instruction(property_type_descriptor, match)

        property_type_descriptor['name'] = TypeNameChecker.require_property(match.group('name'))
        return property_type_descriptor


class StructArrayMemberParserFactory(RegexParserFactory):
    """Factory for creating struct array member parsers"""
    def __init__(self):
        super().__init__(
            r'(?P<name>\S+) = array\((?P<element_type_name>\S+), (?P<byte_size>size=)?(?P<size>\S+)(?:, sort_key=(?P<sort_key>\S+))?\)'
            + OPTIONAL_CONDITIONAL_REGEX_PATTERN,
            StructArrayMemberParser)

# endregion
