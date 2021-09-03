import re
from collections import OrderedDict

from .AliasParser import AliasParserFactory
from .CatsParseException import CatsParseException
from .CommentParser import CommentParser
from .EnumParser import EnumParserFactory
from .ImportParser import ImportParserFactory
from .ScopeManager import ScopeManager
from .StructParser import StructParserFactory


class CatsParser(ScopeManager):
    """Parser used to parse CATS files line by line"""
    def __init__(self, import_resolver):
        super().__init__()
        self.import_resolver = import_resolver

        self.aspect_parser = CommentParser()
        self.type_parser_factories = [
            AliasParserFactory(),
            EnumParserFactory(),
            ImportParserFactory(),
            StructParserFactory()
        ]

        self.wip_type_descriptors = OrderedDict()
        self.active_parser = None

    def process_line(self, line):
        """Processes the next line of input"""
        try:
            self._process_line(line)
        except Exception as ex:
            raise CatsParseException(self.scope(), ex) from ex

    def commit(self):
        """Completes processing of current type"""
        self._close_type()

    def _process_line(self, line):
        self.increment_line_number()

        # check if current line is a cross cutting concern
        line_stripped = line.strip()
        if self.aspect_parser.try_process_line(line_stripped):
            return

        # something else, so attach current aspect state to it
        partial_descriptor = self.aspect_parser.commit()

        # ignore blank lines
        if not line_stripped:
            return

        # close a type iff an unindented non-empty line is found
        if self.active_parser and not line.startswith('\t'):
            self._close_type()

        active_factories = self.type_parser_factories if not self.active_parser else self.active_parser.factories()

        try:
            factory = next(factory for factory in active_factories if factory.is_match(line_stripped))
        except StopIteration as ex:
            raise CatsParseException('none of the parsers matched the line "{}"'.format(line_stripped)) from ex

        parser = factory.create()
        parse_result = parser.process_line(line_stripped)

        # create a new scope if the current symbol is a composite
        if not parse_result:
            self.active_parser = parser
            self.active_parser.partial_descriptor = partial_descriptor
            return

        if self.active_parser:
            if 'type' in parse_result:
                # perform extra validation on some property links for better error detection/messages
                if 'sort_key' in parse_result:
                    # sort key processing will only occur if linked field type already exists
                    self._require_type_with_field(parse_result['type'], parse_result['sort_key'])

                # perform inline struct expansion
                if self._handle_inline_struct({**parse_result, **partial_descriptor}):
                    return

            self.active_parser.append({**parse_result, **partial_descriptor})
        elif hasattr(parse_result, 'import_file'):
            self.import_resolver(parse_result.import_file)
        else:
            self._set_type_descriptor(parse_result[0], {**parse_result[1], **partial_descriptor})

    def _handle_inline_struct(self, parse_result):
        linked_type_name = parse_result['type']
        member_type_descriptor = self._require_known_type(linked_type_name)

        # special primitive types, like byte, do not have explicit type descriptors
        is_inline_struct = member_type_descriptor and 'inline' == member_type_descriptor.get('disposition')
        is_inline_member = 'inline' == parse_result.get('disposition')

        if is_inline_member:
            if not member_type_descriptor:
                raise CatsParseException('type "{}" cannot be inlined'.format(linked_type_name))
        else:
            if is_inline_struct:
                raise CatsParseException('inline struct type "{}" can only be used as a named inline'.format(linked_type_name))

            return False

        if is_inline_struct and not parse_result.get('name'):
            raise CatsParseException('inline struct type "{}" must be named when inlined'.format(linked_type_name))

        if not is_inline_struct:
            if parse_result.get('name'):
                raise CatsParseException('struct type "{}" must be unnamed when inlined'.format(linked_type_name))

            return False

        comment_map = self._build_comment_map(parse_result['comments'])

        for inline_struct_member_descriptor in member_type_descriptor['layout']:
            property_descriptor = {**inline_struct_member_descriptor}
            property_descriptor['comments'] = '\n'.join(comment_map.get(property_descriptor['name'], []))

            property_name = parse_result['name']
            property_descriptor['name'] = self._make_inlined_name(property_name, property_descriptor['name'])

            if property_descriptor.get('condition'):
                property_descriptor['condition'] = self._make_inlined_name(property_name, property_descriptor['condition'])

            if (property_descriptor.get('size')
                    and any(property_descriptor['size'] == descriptor['name'] for descriptor in member_type_descriptor['layout'])):
                property_descriptor['size'] = self._make_inlined_name(property_name, property_descriptor['size'])

            self.active_parser.append(property_descriptor)

        return True

    @staticmethod
    def _make_inlined_name(property_name_prefix, internal_property_name):
        if '__value__' == internal_property_name:
            return property_name_prefix

        return'{}_{}'.format(property_name_prefix, internal_property_name)

    @staticmethod
    def _build_comment_map(comments):
        member_comment_start_regex = re.compile(r'^\[(\S+)\] ')

        comment_map = {}
        active_comment_key = None
        for line in comments.split('\n'):
            member_comment_start_match = member_comment_start_regex.match(line)
            if member_comment_start_match:
                active_comment_key = member_comment_start_match.group(1)
                comment_map[active_comment_key] = [line[len(active_comment_key) + 3:]]
            elif active_comment_key:
                comment_map[active_comment_key].append(line)

        return comment_map

    def _close_type(self):
        if not self.active_parser:
            return

        (new_type_name, new_type_descriptor) = self.active_parser.commit()

        if 'layout' in new_type_descriptor:
            new_type_layout = new_type_descriptor['layout']
            for property_type_descriptor in new_type_layout:
                if 'condition' in property_type_descriptor:
                    # when condition is being post processed here, it is known that the linked condition field is part of
                    # the struct and the linked condition type already exists

                    # look up condition type descriptor by searching active parser descriptor layout
                    condition_field_name = property_type_descriptor['condition']
                    condition_type_descriptor = next(
                        descriptor for descriptor in new_type_layout
                        if 'name' in descriptor and descriptor['name'] == condition_field_name
                    )

                    condition_type_name = condition_type_descriptor['type']
                    condition_value = property_type_descriptor['condition_value']
                    if 'byte' == condition_type_name:
                        if not isinstance(condition_value, int):
                            error_message_format = 'condition value "{}" for "{}" must be numeric'
                            raise CatsParseException(error_message_format.format(condition_value, condition_field_name))
                    else:
                        self._require_enum_type_with_value(condition_type_name, condition_value)

                if any(disposition == property_type_descriptor.get('disposition') for disposition in ['const', 'reserved']):
                    if not isinstance(property_type_descriptor['value'], int):
                        self._require_enum_type_with_value(property_type_descriptor['type'], property_type_descriptor['value'])

                if 'inline' == new_type_descriptor.get('disposition') and 'const' == property_type_descriptor.get('disposition'):
                    error_message_format = 'inline struct "{}" cannot include const field "{}"'
                    raise CatsParseException(error_message_format.format(new_type_name, property_type_descriptor['name']))

        self._set_type_descriptor(new_type_name, {**new_type_descriptor, **self.active_parser.partial_descriptor})
        self.active_parser = None

    def _require_known_type(self, type_name):
        if 'byte' == type_name:
            return None

        if type_name not in self.wip_type_descriptors:
            raise CatsParseException('no definition for linked type "{}"'.format(type_name))

        return self.wip_type_descriptors[type_name]

    def _require_type_with_field(self, type_name, field_name):
        type_descriptor = self.wip_type_descriptors[type_name]
        if not any(field_name == field['name'] for field in type_descriptor['layout']):
            raise CatsParseException('"{}" does not have field "{}"'.format(type_name, field_name))

    def _require_enum_type_with_value(self, type_name, value_name):
        enum_type_descriptor = self.wip_type_descriptors[type_name]
        if 'values' not in enum_type_descriptor:
            raise CatsParseException('linked type "{}" must be an enum type'.format(type_name))

        if not any(value_name == value['name'] for value in enum_type_descriptor['values']):
            raise CatsParseException('linked enum type "{}" does not contain value "{}"'.format(type_name, value_name))

    def _set_type_descriptor(self, type_name, type_descriptor):
        if type_name in self.wip_type_descriptors:
            raise CatsParseException('duplicate definition for type "{}"'.format(type_name))

        self.wip_type_descriptors[type_name] = type_descriptor

    @staticmethod
    def _is_inline_struct(type_descriptor):
        return 'struct' == type_descriptor.get('type') and 'inline' == type_descriptor.get('disposition')

    def type_descriptors(self):
        """Returns all parsed type descriptors"""
        self._close_type()

        # skip inline structs as they're treated as macros and expanded by the parser
        return {
            name: self.wip_type_descriptors[name] for name in self.wip_type_descriptors
            if not self._is_inline_struct(self.wip_type_descriptors[name])
        }
