import unittest

from symbolchain.nem.KeyPair import KeyPair
from symbolchain.nem.SharedKey import SharedKey

from ..test.BasicSharedKeyTest import BasicSharedKeyTest, SharedKeyTestDescriptor

DETERMINISTIC_SALT = b'1234567890ABCDEF1234567890ABCDEF'


class SharedKeyTest(BasicSharedKeyTest, unittest.TestCase):
	@staticmethod
	def get_test_descriptor():
		return SharedKeyTestDescriptor(
			key_pair_class=KeyPair,
			derive_shared_key=SharedKey.derive_shared_key
		)


class SharedKeyDeprecatedTest(BasicSharedKeyTest, unittest.TestCase):
	@staticmethod
	def get_test_descriptor():
		return SharedKeyTestDescriptor(
			key_pair_class=KeyPair,
			derive_shared_key=lambda key_pair, public_key: SharedKey.derive_shared_key_deprecated(key_pair, public_key, DETERMINISTIC_SALT)
		)
