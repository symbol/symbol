import unittest

from symbolchain.core.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.core.nis1.Network import Address
from symbolchain.core.nis1.TypeParsingRules import TypeParsingRules

PUBLIC_KEY_1 = PublicKey('A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74')
PUBLIC_KEY_2 = PublicKey('9A755901AA014A4EACAE615523D2B50C27F954CB936927331F1116C8D5B7B2AA')
YAML_INPUT = '''
- address: TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF
  name: bob
  roles: [green, main]

- public_key: {public_key_1}
  name: TEST
  roles: [red, test]
'''.format(public_key_1=PUBLIC_KEY_1)


class TypeParsingRulesTest(unittest.TestCase):
    # region Address

    def _assert_can_parse_address(self, value, expected_value):
        # Arrange:
        type_parser_map = TypeParsingRules(AccountDescriptorRepository(YAML_INPUT)).as_map()

        # Act:
        value = type_parser_map[Address](value)

        # Assert:
        self.assertEqual(expected_value, value)

    def test_can_parse_named_address_with_overrride(self):
        self._assert_can_parse_address('bob', Address('TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF'))

    def test_cannot_parse_named_address_without_overrride(self):
        # Arrange:
        type_parser_map = TypeParsingRules(AccountDescriptorRepository(YAML_INPUT)).as_map()

        # Act + Assert:
        with self.assertRaises(ValueError):
            type_parser_map[Address]('TEST')

    def test_can_parse_unnamed_address(self):
        self._assert_can_parse_address('TAVOZX4HDVOAR4W6K4WJHWPD3MOFU27DFEJDR2PR', Address('TAVOZX4HDVOAR4W6K4WJHWPD3MOFU27DFEJDR2PR'))

    # endregion

    # region PublicKey

    def _assert_can_parse_public_key(self, value, expected_value):
        # Arrange:
        type_parser_map = TypeParsingRules(AccountDescriptorRepository(YAML_INPUT)).as_map()

        # Act:
        value = type_parser_map[PublicKey](value)

        # Assert:
        self.assertEqual(expected_value, value)

    def test_can_parse_named_public_key_with_overrride(self):
        self._assert_can_parse_public_key('TEST', PUBLIC_KEY_1)

    def test_cannot_parse_named_public_key_without_overrride(self):
        # Arrange:
        type_parser_map = TypeParsingRules(AccountDescriptorRepository(YAML_INPUT)).as_map()

        # Act + Assert:
        with self.assertRaises(ValueError):
            type_parser_map[PublicKey]('bob')

    def test_can_parse_unnamed_public_key(self):
        self._assert_can_parse_public_key(str(PUBLIC_KEY_2), PUBLIC_KEY_2)

    # endregion
