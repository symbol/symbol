import re

from .CatsParseException import CatsParseException

REGEXES = {
    'user_type_name': re.compile(r'^[A-Z][a-zA-Z0-9]*$'),
    'property_name': re.compile(r'^[a-z][a-zA-Z0-9_]*$'),

    'int_or_uint': re.compile(r'^(u)?int(8|16|32|64)$'),
    'binary_fixed_type': re.compile(r'^binary_fixed\((0x[0-9A-F]+|[0-9]+)\)$'),
    'dec_or_hex': re.compile(r'^(0x[0-9A-F]+|[0-9]+)$'),
}


def _match_regex_or_throw(regex_key, line):
    match = REGEXES[regex_key].match(line)
    if not match:
        raise CatsParseException('unable to parse "{0}": {1}'.format(regex_key, line))

    return match


def require_user_type_name(type_name):
    """Raises an exception if the specified name is not a valid user type name"""
    _match_regex_or_throw('user_type_name', type_name)
    return type_name


def require_property_name(type_name):
    """Raises an exception if the specified name is not a valid property name"""
    _match_regex_or_throw('property_name', type_name)
    return type_name


def is_primitive(type_name):
    """Returns true if the specified name is a valid primitive name"""
    return REGEXES['int_or_uint'].match(type_name)


def require_primitive(type_name):
    """Raises an exception if the specified name is not a valid primitive name"""
    _match_regex_or_throw('int_or_uint', type_name)
    return type_name


def is_dec_or_hex(string):
    """Returns true if the specified string is a valid decimal or hexidecimal number"""
    return REGEXES['dec_or_hex'].match(string)


def parse_dec_or_hex(string):
    """Parses a string as either decimal or hexidecimal"""
    base = 16 if string.startswith('0x') else 10
    return int(string, base)


def is_builtin(type_name):
    return REGEXES['int_or_uint'].match(type_name) or REGEXES['binary_fixed_type'].match(type_name)


def parse_builtin(type_name):
    """Parses a builtin type, either binary_fixed or a uint alias"""
    is_unsigned = True
    binary_fixed_type_match = REGEXES['binary_fixed_type'].match(type_name)
    if binary_fixed_type_match:
        type_descriptor = {'size': parse_dec_or_hex(binary_fixed_type_match.group(1))}
    else:
        match = _match_regex_or_throw('int_or_uint', type_name)
        is_unsigned = bool(match.group(1))
        uint_byte_count = int(match.group(2)) // 8
        type_descriptor = {'size': uint_byte_count}

    return {**type_descriptor, 'type': 'byte', 'signedness': 'unsigned' if is_unsigned else 'signed'}
