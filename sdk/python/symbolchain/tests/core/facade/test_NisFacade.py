import unittest
from binascii import unhexlify

from symbolchain.core.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.core.CryptoTypes import Hash256, PrivateKey, PublicKey
from symbolchain.core.facade.NisFacade import NisFacade
from symbolchain.tests.test.NemTestUtils import NemTestUtils

YAML_INPUT = '''
- public_key: A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74
'''


class NisFacadeTest(unittest.TestCase):
    # region constants / hasher

    def test_key_pair_is_correct(self):
        # Arrange:
        private_key = PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57')  # reversed

        # Act:
        key_pair = NisFacade.KeyPair(private_key)

        # Assert:
        self.assertEqual(PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0'), key_pair.public_key)

    def test_can_sign_and_verify(self):
        # Arrange:
        private_key = PrivateKey.random()
        key_pair = NisFacade.KeyPair(private_key)
        message = NemTestUtils.randbytes(21)

        # Act:
        signature = key_pair.sign(message)
        is_verified = NisFacade.Verifier(key_pair.public_key).verify(message, signature)

        # Assert:
        self.assertTrue(is_verified)

    def test_hasher_is_correct(self):
        # Arrange:
        message = ''.join([
            'A6151D4904E18EC288243028CEDA30556E6C42096AF7150D6A7232CA5DBA52BD',
            '2192E23DAA5FA2BEA3D4BD95EFA2389CD193FCD3376E70A5C097B32C1C62C80A',
            'F9D710211545F7CDDDF63747420281D64529477C61E721273CFD78F8890ABB40',
            '70E97BAA52AC8FF61C26D195FC54C077DEF7A3F6F79B36E046C1A83CE9674BA1',
            '983EC2FB58947DE616DD797D6499B0385D5E8A213DB9AD5078A8E0C940FF0CB6',
            'BF92357EA5609F778C3D1FB1E7E36C35DB873361E2BE5C125EA7148EFF4A035B',
            '0CCE880A41190B2E22924AD9D1B82433D9C023924F2311315F07B88BFD428500',
            '47BF3BE785C4CE11C09D7E02065D30F6324365F93C5E7E423A07D754EB314B5F',
            'E9DB4614275BE4BE26AF017ABDC9C338D01368226FE9AF1FB1F815E7317BDBB3',
            '0A0F36DC69'
        ])

        # Act:
        hash_value = NisFacade.hash_buffer(unhexlify(message))

        # Assert:
        self.assertEqual(Hash256('4E9E79AB7434F6C7401FB3305D55052EE829B9E46D5D05D43B59FEFB32E9A619'), hash_value)

    # endregion

    # region constructor

    def test_can_create_around_known_network(self):
        # Act:
        facade = NisFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
        transaction = facade.transaction_factory.create('transfer')

        # Assert:
        self.assertEqual(0x0101, transaction.type)
        self.assertEqual(0x98000001, transaction.version)

    def test_cannot_create_around_unknown_network(self):
        # Act:
        with self.assertRaises(StopIteration):
            NisFacade('foo', AccountDescriptorRepository(YAML_INPUT))

    # endregion
