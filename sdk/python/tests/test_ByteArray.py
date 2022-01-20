import unittest
from binascii import hexlify

from symbolchain.ByteArray import ByteArray

from .test.ComparisonTestUtils import ComparisonTestDescriptor, ComparisonTestUtils, EqualityTestDescriptor
from .test.TestUtils import TestUtils

FIXED_SIZE = 24
TEST_BYTES = bytes([
	0xC5, 0xFB, 0x65, 0xCB, 0x90, 0x26, 0x23, 0xD9,
	0x3D, 0xF2, 0xE6, 0x82, 0xFF, 0xB1, 0x3F, 0x99,
	0xD5, 0x0F, 0xAC, 0x24, 0xD5, 0xFF, 0x2A, 0x42
])
TEST_HEX = 'C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42'


DEFAULT_VALUE = 0x12345678_09ABCDEF


def int_to_bytes24(value):
	return value.to_bytes(8, 'little') + bytes(FIXED_SIZE - 8)


class FakeByteArray:
	def __init__(self, value):
		self.bytes = value


def random_hex_string(size):
	return hexlify(TestUtils.randbytes(size)).decode('utf8')


class ByteArrayTest(ComparisonTestUtils, unittest.TestCase):
	DESCRIPTOR = ComparisonTestDescriptor(
		lambda value: ByteArray(FIXED_SIZE, int_to_bytes24(value)),
		lambda value: ByteArray(FIXED_SIZE, int_to_bytes24(value), str),
		FakeByteArray,
	)

	def test_can_create_byte_array_with_correct_number_of_bytes(self):
		# Act:
		byte_array = ByteArray(FIXED_SIZE, TEST_BYTES)

		# Assert:
		self.assertEqual(TEST_BYTES, byte_array.bytes)

	def test_cannot_create_byte_array_with_incorrect_number_of_bytes(self):
		for size in [0, FIXED_SIZE - 1, FIXED_SIZE + 1]:
			with self.assertRaises(ValueError):
				ByteArray(FIXED_SIZE, TestUtils.randbytes(size))

	def test_can_create_byte_array_with_correct_number_of_hex_characters(self):
		# Act:
		byte_array = ByteArray(FIXED_SIZE, TEST_HEX)

		# Assert:
		self.assertEqual(TEST_BYTES, byte_array.bytes)

	def test_cannot_create_byte_array_with_incorrect_number_of_hex_characters(self):
		for size in [0, FIXED_SIZE - 1, FIXED_SIZE + 1]:
			with self.assertRaises(ValueError):
				ByteArray(FIXED_SIZE, random_hex_string(size))

	def test_equality_and_inequality_are_supported(self):
		# Arrange:
		descriptor = EqualityTestDescriptor(
			lambda value: ByteArray(FIXED_SIZE, value),
			lambda value: ByteArray(FIXED_SIZE, value, str),
			None,
			FakeByteArray,
		)

		# Act + Assert:
		descriptor.random = lambda: TestUtils.randbytes(FIXED_SIZE)
		self.equality_is_supported(descriptor, TEST_BYTES)
		self.inequality_is_supported(descriptor, TEST_BYTES)

		descriptor.random = lambda: random_hex_string(FIXED_SIZE)
		self.equality_is_supported(descriptor, TEST_HEX)
		self.inequality_is_supported(descriptor, TEST_HEX)

	def test_less_than_is_supported(self):
		self.less_than_is_supported(self.DESCRIPTOR, DEFAULT_VALUE)

	def test_less_than_equal_is_supported(self):
		self.less_than_equal_is_supported(self.DESCRIPTOR, DEFAULT_VALUE)

	def test_greater_than_is_supported(self):
		self.greater_than_is_supported(self.DESCRIPTOR, DEFAULT_VALUE)

	def test_greater_than_equal_is_supported(self):
		self.greater_than_equal_is_supported(self.DESCRIPTOR, DEFAULT_VALUE)

	def test_hash_is_supported(self):
		# Arrange:
		byte_array_hash = hash(ByteArray(FIXED_SIZE, TEST_BYTES))

		# Act + Assert:
		self.assertEqual(byte_array_hash, hash(ByteArray(FIXED_SIZE, TEST_BYTES)))
		self.assertEqual(byte_array_hash, hash(ByteArray(FIXED_SIZE, TEST_HEX)))

		self.assertNotEqual(byte_array_hash, hash(ByteArray(FIXED_SIZE, TestUtils.randbytes(FIXED_SIZE))))
		self.assertNotEqual(byte_array_hash, hash(ByteArray(FIXED_SIZE, random_hex_string(FIXED_SIZE))))
		self.assertNotEqual(byte_array_hash, None)

	def test_string_is_supported(self):
		self.assertEqual(TEST_HEX, str(ByteArray(FIXED_SIZE, TEST_BYTES)))
