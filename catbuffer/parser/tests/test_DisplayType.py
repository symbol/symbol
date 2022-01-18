import unittest

from catparser.DisplayType import DisplayType


class DisplayTypeTests(unittest.TestCase):
	def test_is_array_returns_correct_values(self):
		self.assertEqual(False, DisplayType.UNSET.is_array)
		self.assertEqual(False, DisplayType.INTEGER.is_array)
		self.assertEqual(True, DisplayType.BYTE_ARRAY.is_array)
		self.assertEqual(True, DisplayType.TYPED_ARRAY.is_array)
		self.assertEqual(False, DisplayType.ENUM.is_array)
		self.assertEqual(False, DisplayType.STRUCT.is_array)
