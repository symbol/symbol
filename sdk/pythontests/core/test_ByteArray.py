import unittest
from binascii import hexlify

from symbolchain.core.ByteArray import ByteArray

from ..test.NemTestUtils import NemTestUtils

FIXED_SIZE = 24
TEST_BYTES = bytes([
    0xC5, 0xFB, 0x65, 0xCB, 0x90, 0x26, 0x23, 0xD9,
    0x3D, 0xF2, 0xE6, 0x82, 0xFF, 0xB1, 0x3F, 0x99,
    0xD5, 0x0F, 0xAC, 0x24, 0xD5, 0xFF, 0x2A, 0x42
])
TEST_HEX = 'C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42'


def random_hex_string(size):
    return hexlify(NemTestUtils.randbytes(size)).decode('utf8')


class ByteArrayTest(unittest.TestCase):
    def test_can_create_byte_array_with_correct_number_of_bytes(self):
        # Act:
        byte_array = ByteArray(FIXED_SIZE, TEST_BYTES)

        # Assert:
        self.assertEqual(TEST_BYTES, byte_array.bytes)

    def test_cannot_create_byte_array_with_incorrect_number_of_bytes(self):
        for size in [0, FIXED_SIZE - 1, FIXED_SIZE + 1]:
            with self.assertRaises(ValueError):
                ByteArray(FIXED_SIZE, NemTestUtils.randbytes(size))

    def test_can_create_byte_array_with_correct_number_of_hex_characters(self):
        # Act:
        byte_array = ByteArray(FIXED_SIZE, TEST_HEX)

        # Assert:
        self.assertEqual(TEST_BYTES, byte_array.bytes)

    def test_cannot_create_byte_array_with_incorrect_number_of_hex_characters(self):
        for size in [0, FIXED_SIZE - 1, FIXED_SIZE + 1]:
            with self.assertRaises(ValueError):
                ByteArray(FIXED_SIZE, random_hex_string(size))

    def test_equality_is_supported(self):
        # Arrange:
        byte_array = ByteArray(FIXED_SIZE, TEST_BYTES)
        byte_array_with_explicit_tag = ByteArray(FIXED_SIZE, TEST_BYTES, str)

        # Act + Assert:
        self.assertEqual(byte_array, ByteArray(FIXED_SIZE, TEST_BYTES))
        self.assertEqual(byte_array, ByteArray(FIXED_SIZE, TEST_HEX))
        self.assertEqual(byte_array_with_explicit_tag, ByteArray(FIXED_SIZE, TEST_BYTES, str))
        self.assertEqual(byte_array_with_explicit_tag, ByteArray(FIXED_SIZE, TEST_HEX, str))

        self.assertNotEqual(byte_array, ByteArray(FIXED_SIZE, NemTestUtils.randbytes(FIXED_SIZE)))
        self.assertNotEqual(byte_array, ByteArray(FIXED_SIZE, random_hex_string(FIXED_SIZE)))
        self.assertNotEqual(byte_array, byte_array_with_explicit_tag)
        self.assertNotEqual(byte_array, None)

    def test_string_is_supported(self):
        self.assertEqual(TEST_HEX, str(ByteArray(FIXED_SIZE, TEST_BYTES)))
