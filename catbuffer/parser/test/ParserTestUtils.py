from catbuffer_parser.CatsParseException import CatsParseException


class SingleLineParserTestUtils:
    def __init__(self, parser_factory_type, unittest):
        self.parser_factory_type = parser_factory_type
        self.unittest = unittest

    def assert_parse(self, line, expected_result):
        # Arrange:
        parser = self.parser_factory_type().create()

        # Act:
        result = parser.process_line(line)

        # Assert:
        self.unittest.assertEqual(expected_result, result)

    def assert_parse_exception(self, line, exception_type=CatsParseException):
        # Arrange:
        parser = self.parser_factory_type().create()

        # Sanity:
        self.unittest.assertTrue(self.parser_factory_type().is_match(line))

        # Act + Assert:
        with self.unittest.assertRaises(exception_type):
            parser.process_line(line)

    def assert_parse_exceptions(self, invalid_lines):
        # Arrange:
        for line in invalid_lines:
            # Act + Assert:
            self.assert_parse_exception(line)

    def assert_naming(self, pattern, valid_names, invalid_names):
        for name in valid_names:
            # Act + Assert: no exception
            self.parser_factory_type().create().process_line(pattern.format(name))

        for name in invalid_names:
            # Act + Assert: exception
            self.assert_parse_exception(pattern.format(name))


class MultiLineParserTestUtils(SingleLineParserTestUtils):
    def assert_parse(self, line, expected_result):
        # Arrange:
        parser = self.parser_factory_type().create()

        # Act:
        parser.process_line(line)
        result = parser.commit()

        # Assert:
        self.unittest.assertEqual(expected_result, result)


class ParserFactoryTestUtils:
    def __init__(self, parser_factory_type, unittest):
        self.parser_factory_type = parser_factory_type
        self.unittest = unittest

    def assert_positives(self, matches):
        # Arrange:
        factory = self.parser_factory_type()

        # Act + Assert:
        for line in matches:
            self.unittest.assertTrue(factory.is_match(line))

    def assert_negatives(self, matches):
        # Arrange:
        factory = self.parser_factory_type()

        # Act + Assert:
        for line in matches:
            self.unittest.assertFalse(factory.is_match(line))
