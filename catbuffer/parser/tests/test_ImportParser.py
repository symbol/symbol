import unittest

from catparser.ImportParser import ImportParserFactory, ImportResult

from .ParserTestUtils import ParserFactoryTestUtils, SingleLineParserTestUtils

VALID_IMPORT_FILE_NAMES = ['A', 'aBzZzac09', 'aa bb.cats', 'foo.cats', 'foo bar', 'foo bar.cats', '$^$']


class ImportParserFactoryTest(unittest.TestCase):
    def test_is_match_returns_true_for_positives(self):
        ParserFactoryTestUtils(ImportParserFactory, self).assert_positives([
            'import "{}"'.format(import_file) for import_file in VALID_IMPORT_FILE_NAMES
        ])

    def test_is_match_returns_false_for_negatives(self):
        ParserFactoryTestUtils(ImportParserFactory, self).assert_negatives([
            ' import "A"', 'import "A" ', 'import ""', 'import "aa\taa"', 'foo $$$'
        ])


class ImportParserTest(unittest.TestCase):
    def test_can_parse_import(self):
        for import_file in VALID_IMPORT_FILE_NAMES:
            SingleLineParserTestUtils(ImportParserFactory, self).assert_parse(
                'import "{}"'.format(import_file),
                ImportResult(import_file))
