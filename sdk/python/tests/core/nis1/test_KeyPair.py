import unittest

from symbolchain.core.CryptoTypes import PrivateKey, PublicKey
from symbolchain.core.nis1.KeyPair import KeyPair, Verifier

from ...test.BasicKeyPairTest import BasicKeyPairTest, KeyPairTestDescriptor


class NetworkTest(BasicKeyPairTest, unittest.TestCase):
    def get_test_descriptor(self):
        deterministic_private_key = PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57')  # reversed
        expected_public_key = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')
        return KeyPairTestDescriptor(KeyPair, Verifier, deterministic_private_key, expected_public_key)
