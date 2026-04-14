import unittest

from symbolchain.symbol.Restriction import mosaic_restriction_generate_key


class RestrictionTest(unittest.TestCase):
	def _assert_key_generation(self, seed, expected_key):
		# Act:
		key = mosaic_restriction_generate_key(seed)

		# Assert:
		self.assertEqual(expected_key, key)

	def test_can_generate_expected_keys_from_seeds(self):
		self._assert_key_generation('a', 0x7524A0FBF24B0880)  # unlike metadata_generate_key, high bit can be unset
		self._assert_key_generation('abc', 0xB225E24FA75D983A)
		self._assert_key_generation('def', 0xB0AC5222678F0D8E)
