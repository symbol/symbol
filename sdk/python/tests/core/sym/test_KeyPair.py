import unittest

from symbolchain.core.CryptoTypes import PrivateKey, PublicKey
from symbolchain.core.sym.KeyPair import KeyPair, Verifier

from ...test.BasicKeyPairTest import BasicKeyPairTest, KeyPairTestDescriptor


class NetworkTest(BasicKeyPairTest, unittest.TestCase):
    def get_test_descriptor(self):
        deterministic_private_key = PrivateKey('E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E')
        expected_public_key = PublicKey('E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD')
        return KeyPairTestDescriptor(KeyPair, Verifier, deterministic_private_key, expected_public_key)
