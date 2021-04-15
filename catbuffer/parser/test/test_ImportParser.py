import unittest
from test.ParserTestUtils import ParserFactoryTestUtils, SingleLineParserTestUtils

from catbuffer_parser.ImportParser import ImportParserFactory, ImportResult

VALID_IMPORT_FILE_NAMES = ['A', 'aBzZzac09', 'aa bb.cats', 'foo.cats', 'foo bar', 'foo bar.cats', '$^$']


class ImportParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        # Assert:
        ParserFactoryTestUtils(ImportParserFactory, self).assert_positives([
            'import "{0}"'.format(import_file) for import_file in VALID_IMPORT_FILE_NAMES
        ])

    def test_is_match_returns_false_for_negatives(self):
        # Assert:
        ParserFactoryTestUtils(ImportParserFactory, self).assert_negatives([
            ' import "A"', 'import "A" ', 'import ""', 'import "aa\taa"', 'foo $$$'
        ])


class ImportParserTest(unittest.TestCase):
    def test_can_parse_import(self):
        for import_file in VALID_IMPORT_FILE_NAMES:
            # Act + Assert:
            SingleLineParserTestUtils(ImportParserFactory, self).assert_parse(
                'import "{0}"'.format(import_file),
                ImportResult(import_file))
