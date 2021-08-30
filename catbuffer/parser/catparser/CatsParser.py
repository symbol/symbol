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
            raise CatsParseException('\n'.join(self.scope()), ex) from ex

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
            raise CatsParseException('none of the parsers matched the line "{0}"'.format(line_stripped)) from ex

        parser = factory.create()
        parse_result = parser.process_line(line_stripped)

        # create a new scope if the current symbol is a composite
        if not parse_result:
            self.active_parser = parser
            self.active_parser.partial_descriptor = partial_descriptor
            return

        if self.active_parser:
            if 'type' in parse_result:
                self._require_known_type(parse_result['type'])

                # perform extra validation on some property links for better error detection/messages
                if 'sort_key' in parse_result:
                    # sort key processing will only occur if linked field type already exists
                    self._require_type_with_field(parse_result['type'], parse_result['sort_key'])

            self.active_parser.append({**parse_result, **partial_descriptor})
        elif hasattr(parse_result, 'import_file'):
            self.import_resolver(parse_result.import_file)
        else:
            self._set_type_descriptor(parse_result[0], {**parse_result[1], **partial_descriptor})

    def _close_type(self):
        if not self.active_parser:
            return

        parsed_tuple = self.active_parser.commit()

        if 'layout' in parsed_tuple[1]:
            new_type_layout = parsed_tuple[1]['layout']
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

                    self._require_enum_type_with_value(condition_type_descriptor['type'], property_type_descriptor['condition_value'])

        self._set_type_descriptor(parsed_tuple[0], {**parsed_tuple[1], **self.active_parser.partial_descriptor})
        self.active_parser = None

    def _require_known_type(self, type_name):
        if type_name not in self.wip_type_descriptors and 'byte' != type_name:
            raise CatsParseException('no definition for linked type "{0}"'.format(type_name))

        return type_name

    def _require_type_with_field(self, type_name, field_name):
        type_descriptor = self.wip_type_descriptors[type_name]
        if not any(field_name == field['name'] for field in type_descriptor['layout']):
            raise CatsParseException('"{0}" does not have field "{1}"'.format(type_name, field_name))

    def _require_enum_type_with_value(self, type_name, value_name):
        enum_type_descriptor = self.wip_type_descriptors[type_name]
        if 'values' not in enum_type_descriptor:
            raise CatsParseException('linked type "{0}" must be an enum type'.format(type_name))

        if not any(value_name == value['name'] for value in enum_type_descriptor['values']):
            raise CatsParseException('linked enum type "{0}" does not contain value "{1}"'.format(type_name, value_name))

    def _set_type_descriptor(self, type_name, type_descriptor):
        if type_name in self.wip_type_descriptors:
            raise CatsParseException('duplicate definition for type "{0}"'.format(type_name))

        self.wip_type_descriptors[type_name] = type_descriptor

    def type_descriptors(self):
        """Returns all parsed type descriptors"""
        self._close_type()
        return self.wip_type_descriptors
