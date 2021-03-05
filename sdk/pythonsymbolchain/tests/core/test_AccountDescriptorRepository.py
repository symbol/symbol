import unittest

from symbolchain.core.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.core.CryptoTypes import PublicKey
from symbolchain.tests.test.NemTestUtils import NemTestUtils

PUBLIC_KEY_1 = PublicKey('A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74')
PUBLIC_KEY_2 = PublicKey('9A755901AA014A4EACAE615523D2B50C27F954CB936927331F1116C8D5B7B2AA')
YAML_INPUT = '''
- address: TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF
  name: alice
  roles: [green, main]

- public_key: {public_key_1}
  name: TEST1
  roles: [red, test]

- public_key: {public_key_2}
  name: BOB
  roles: [BLUE, main]

- address:  TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C
  name: charlie
'''.format(public_key_1=PUBLIC_KEY_1, public_key_2=PUBLIC_KEY_2)


class AccountDescriptorRepositoryTest(unittest.TestCase):
    def test_can_load_descriptors_file(self):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Assert:
        self.assertEqual(4, len(repository.descriptors))

    def test_cannot_find_by_name_when_no_match(self):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Act:
        descriptor = repository.try_find_by_name('dave')

        # Assert:
        self.assertEqual(None, descriptor)

    def test_can_find_by_name_when_match(self):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Act
        descriptor1 = repository.try_find_by_name('alice')
        descriptor2 = repository.try_find_by_name('TEST1')
        descriptor3 = repository.try_find_by_name('BOB')
        descriptor4 = repository.try_find_by_name('charlie')

        # Assert:
        self.assertEqual(None, descriptor1.public_key)
        self.assertEqual('TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF', descriptor1.address)
        self.assertEqual('alice', descriptor1.name)
        self.assertEqual(['green', 'main'], descriptor1.roles)

        self.assertEqual(PUBLIC_KEY_1, descriptor2.public_key)
        self.assertEqual(None, descriptor2.address)
        self.assertEqual('TEST1', descriptor2.name)
        self.assertEqual(['red', 'test'], descriptor2.roles)

        self.assertEqual(PUBLIC_KEY_2, descriptor3.public_key)
        self.assertEqual(None, descriptor3.address)
        self.assertEqual('BOB', descriptor3.name)
        self.assertEqual(['BLUE', 'main'], descriptor3.roles)

        self.assertEqual(None, descriptor4.public_key)
        self.assertEqual('TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C', descriptor4.address)
        self.assertEqual('charlie', descriptor4.name)
        self.assertEqual([], descriptor4.roles)

    def test_cannot_find_by_public_key_when_no_match(self):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Act + Assert:
        with self.assertRaises(StopIteration):
            repository.find_by_public_key(PublicKey(NemTestUtils.randbytes(32)))

    def test_can_find_by_public_key_when_match(self):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Act:
        descriptor1 = repository.find_by_public_key(PUBLIC_KEY_1)
        descriptor2 = repository.find_by_public_key(PUBLIC_KEY_2)

        # Assert:
        self.assertEqual('TEST1', descriptor1.name)
        self.assertEqual('BOB', descriptor2.name)

    def _assert_can_find_all_by_role(self, role, expected_match_names):
        # Arrange:
        repository = AccountDescriptorRepository(YAML_INPUT)

        # Act:
        descriptors = repository.find_all_by_role(role)
        match_names = [descriptor.name for descriptor in descriptors]

        # Assert:
        self.assertEqual(expected_match_names, match_names)

    def test_find_all_by_role_when_no_match(self):
        self._assert_can_find_all_by_role('blue', [])

    def test_find_all_by_role_when_single_match(self):
        self._assert_can_find_all_by_role('test', ['TEST1'])

    def test_find_all_by_role_when_multiple_matches(self):
        self._assert_can_find_all_by_role('main', ['alice', 'BOB'])

    def test_find_all_by_role_when_no_role_filter_is_provided(self):
        self._assert_can_find_all_by_role(None, ['alice', 'TEST1', 'BOB', 'charlie'])
