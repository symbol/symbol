import unittest

from symbolchain.symbol.Metadata import metadata_generate_key, metadata_update_value


class MetadataTest(unittest.TestCase):
	def _assert_key_generation(self, seed, expected_key):
		# Act:
		key = metadata_generate_key(seed)

		# Assert:
		self.assertEqual(expected_key, key)

	def test_can_generate_expected_keys_from_seeds(self):
		self._assert_key_generation('a', 0xF524A0FBF24B0880)
		self._assert_key_generation('abc', 0xB225E24FA75D983A)
		self._assert_key_generation('def', 0xB0AC5222678F0D8E)

	def test_can_set_new_value_without_old_value(self):
		# Arrange:
		new_value = [0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]

		# Act:
		result = metadata_update_value(None, new_value)

		# Assert:
		self.assertEqual(new_value, result)

	def test_can_update_equal_length_value(self):
		# Arrange:
		old_value = [0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]
		new_value = [0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78]

		# Act:
		result = metadata_update_value(old_value, new_value)

		# Assert:
		self.assertEqual(bytes([0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78]), result)

	def test_can_update_shorter_value(self):
		# Arrange:
		old_value = [0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]
		new_value = [0xD4, 0x60, 0x82, 0xF8]

		# Act:
		result = metadata_update_value(old_value, new_value)

		# Assert:
		self.assertEqual(bytes([0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7, 0xB0, 0x36]), result)

	def test_can_update_longer_value(self):
		# Arrange:
		old_value = [0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]
		new_value = [0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78, 0xE6, 0x9D, 0xD6]

		# Act:
		result = metadata_update_value(old_value, new_value)

		# Assert:
		self.assertEqual(
			bytes([0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78, 0xE6, 0x9D, 0xD6]),
			result)
