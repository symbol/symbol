import unittest

from symbolchain.core.BlockchainSettings import BlockchainSettings
from symbolchain.core.CryptoTypes import PublicKey

PUBLIC_KEY_1 = PublicKey('A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74')
PUBLIC_KEY_2 = PublicKey('9A755901AA014A4EACAE615523D2B50C27F954CB936927331F1116C8D5B7B2AA')
YAML_INPUT = '''
blockchain: nis1
network: testnet

nodes:
  - host: alice
    roles: [historical, XXL]

  - host: bob
    roles: [backup, super]

  - host: charlie

accounts:
  - public_key: {public_key_1}
    name: bobby
    roles: [green, main]

  - public_key: {public_key_2}
    name: TEST1
    roles: [red, test]
'''.format(public_key_1=PUBLIC_KEY_1, public_key_2=PUBLIC_KEY_2)


class BlockchainSettingsTest(unittest.TestCase):
    def test_can_load_settings_file(self):
        # Arrange:
        settings = BlockchainSettings.load_from_yaml(YAML_INPUT)

        # Assert:
        self.assertEqual('nis1', settings.blockchain)
        self.assertEqual('testnet', settings.network)

        self.assertEqual(3, len(settings.nodes.descriptors))
        self.assertEqual(['alice', 'bob', 'charlie'], [descriptor.host for descriptor in settings.nodes.descriptors])

        self.assertEqual(2, len(settings.accounts.descriptors))
        self.assertEqual(PUBLIC_KEY_1, settings.accounts.try_find_by_name('bobby').public_key)
        self.assertEqual(PUBLIC_KEY_2, settings.accounts.try_find_by_name('TEST1').public_key)
