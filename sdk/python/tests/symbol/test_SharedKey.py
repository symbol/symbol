import unittest

from symbolchain.symbol.KeyPair import KeyPair
from symbolchain.symbol.SharedKey import SharedKey

from ..test.BasicSharedKeyTest import BasicSharedKeyTest, SharedKeyTestDescriptor


class SharedKeyTest(BasicSharedKeyTest, unittest.TestCase):
	@staticmethod
	def get_test_descriptor():
		return SharedKeyTestDescriptor(
			key_pair_class=KeyPair,
			derive_shared_key=SharedKey.derive_shared_key
		)
