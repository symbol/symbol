import unittest

from test.test.BasicKeyPairTest import BasicKeyPairTest, KeyPairTestDescriptor
from core.CryptoTypes import PrivateKey, PublicKey
from nis1.KeyPair import KeyPair, Verifier


class NetworkTest(BasicKeyPairTest, unittest.TestCase):
    def get_test_descriptor(self):
        deterministic_private_key = PrivateKey('575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED')
        expected_public_key = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')
        return KeyPairTestDescriptor(KeyPair, Verifier, deterministic_private_key, expected_public_key)
