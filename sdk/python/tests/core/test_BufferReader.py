import unittest

from symbolchain.core.BufferReader import BufferReader


class BufferReaderTest(unittest.TestCase):
    def test_initially_at_eof_when_buffer_empty(self):
        # Act:
        reader = BufferReader(bytes())

        # Assert:
        self.assertTrue(reader.eof)

    def test_initially_not_at_eof_when_buffer_not_empty(self):
        # Act:
        reader = BufferReader(bytes([0xFF]))

        # Assert:
        self.assertFalse(reader.eof)

    def test_eof_only_when_buffer_fully_read(self):
        # Arrange:
        reader = BufferReader(bytes([0xFF, 0xFF, 0xFF, 0xFF]))

        # Act + Assert: partial read
        reader.read_bytes(3)
        self.assertFalse(reader.eof)

        # Act + Assert: full read
        reader.read_bytes(1)
        self.assertTrue(reader.eof)

    def _assert_can_read_int(self, reader):
        # Act:
        read_int1 = reader.read_int(4)
        read_int2 = reader.read_int(2)
        read_int3 = reader.read_int(2)

        # Assert:
        self.assertTrue(reader.eof)
        self.assertEqual(0x05010203, read_int1)
        self.assertEqual(0x00FF, read_int2)
        self.assertEqual(0xEE00, read_int3)

    def test_can_read_int_endianness_default(self):
        self._assert_can_read_int(BufferReader([0x03, 0x02, 0x01, 0x05, 0xFF, 0x00, 0x00, 0xEE]))

    def test_can_read_int_endianness_little(self):
        self._assert_can_read_int(BufferReader([0x03, 0x02, 0x01, 0x05, 0xFF, 0x00, 0x00, 0xEE], 'little'))

    def test_can_read_int_endianness_big(self):
        self._assert_can_read_int(BufferReader([0x05, 0x01, 0x02, 0x03, 0x00, 0xFF, 0xEE, 0x00], 'big'))

    def test_can_read_string(self):
        # Arrange:
        reader = BufferReader(bytes([0x62, 0x61, 0x72]))

        # Act:
        read_str = reader.read_string(3)

        # Assert:
        self.assertTrue(reader.eof)
        self.assertEqual('bar', read_str)

    def test_can_read_hex_string(self):
        # Arrange:
        reader = BufferReader(bytes([0xC0, 0xAA, 0x00, 0x08]))

        # Act:
        read_hex_str = reader.read_hex_string(4)

        # Assert:
        self.assertTrue(reader.eof)
        self.assertEqual('C0AA0008', read_hex_str)

    def test_can_read_bytes(self):
        # Arrange:
        reader = BufferReader(bytes([0x99, 0x00, 0x40, 0x30, 0x78]))

        # Act:
        read_bytes = reader.read_bytes(5)

        # Assert:
        self.assertTrue(reader.eof)
        self.assertEqual(bytes([0x99, 0x00, 0x40, 0x30, 0x78]), read_bytes)

    def _assert_can_read_all(self, reader):
        # Act:
        read_str = reader.read_string(3)
        read_int = reader.read_int(4)
        read_bytes = reader.read_bytes(5)
        read_hex_string = reader.read_hex_string(4)

        # Assert:
        self.assertTrue(reader.eof)
        self.assertEqual('bar', read_str)
        self.assertEqual(0x05010203, read_int)
        self.assertEqual(bytes([0x99, 0x00, 0x40, 0x30, 0x78]), read_bytes)
        self.assertEqual('C0AA0008', read_hex_string)

    def test_can_read_all_endianness_default(self):
        self._assert_can_read_all(BufferReader(bytes([
            0x62, 0x61, 0x72,
            0x03, 0x02, 0x01, 0x05,
            0x99, 0x00, 0x40, 0x30, 0x78,
            0xC0, 0xAA, 0x00, 0x08
        ])))

    def test_can_read_all_endianness_little(self):
        self._assert_can_read_all(BufferReader(bytes([
            0x62, 0x61, 0x72,
            0x03, 0x02, 0x01, 0x05,
            0x99, 0x00, 0x40, 0x30, 0x78,
            0xC0, 0xAA, 0x00, 0x08
        ]), 'little'))

    def test_can_read_all_endianness_big(self):
        self._assert_can_read_all(BufferReader(bytes([
            0x62, 0x61, 0x72,
            0x05, 0x01, 0x02, 0x03,
            0x99, 0x00, 0x40, 0x30, 0x78,
            0xC0, 0xAA, 0x00, 0x08
        ]), 'big'))
