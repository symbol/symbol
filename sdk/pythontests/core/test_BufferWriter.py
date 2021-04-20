import unittest

from symbolchain.core.BufferWriter import BufferWriter


class BufferWriterTest(unittest.TestCase):
    def test_output_buffer_is_initially_empty(self):
        # Act:
        writer = BufferWriter()

        # Assert:
        self.assertEqual(bytes(), writer.buffer)

    def _assert_can_write_int_endianness_little(self, writer):
        # Act:
        writer.write_int(0x05010203, 4)
        writer.write_int(0x00FF, 2)
        writer.write_int(0xEE00, 2)

        # Assert:
        self.assertEqual(bytes([0x03, 0x02, 0x01, 0x05, 0xFF, 0x00, 0x00, 0xEE]), writer.buffer)

    def test_can_write_int_endianness_default(self):
        self._assert_can_write_int_endianness_little(BufferWriter())

    def test_can_write_int_endianness_little(self):
        self._assert_can_write_int_endianness_little(BufferWriter('little'))

    def test_can_write_int_endianness_big(self):
        # Arrange:
        writer = BufferWriter('big')

        # Act:
        writer.write_int(0x05010203, 4)
        writer.write_int(0x00FF, 2)
        writer.write_int(0xEE00, 2)

        # Assert:
        self.assertEqual(bytes([0x05, 0x01, 0x02, 0x03, 0x00, 0xFF, 0xEE, 0x00]), writer.buffer)

    def test_can_write_string(self):
        # Arrange:
        writer = BufferWriter()

        # Act:
        writer.write_string('bar')

        # Assert:
        self.assertEqual(bytes([0x62, 0x61, 0x72]), writer.buffer)

    def test_can_write_hex_string(self):
        # Arrange:
        writer = BufferWriter()

        # Act:
        writer.write_hex_string('C0AA0008')

        # Assert:
        self.assertEqual(bytes([0xC0, 0xAA, 0x00, 0x08]), writer.buffer)

    def test_can_write_bytes(self):
        # Arrange:
        writer = BufferWriter()

        # Act:
        writer.write_bytes(bytes([0x99, 0x00, 0x40, 0x30, 0x78]))

        # Assert:
        self.assertEqual(bytes([0x99, 0x00, 0x40, 0x30, 0x78]), writer.buffer)

    def _assert_can_write_all_endianness_little(self, writer):
        # Act:
        writer.write_string('bar')
        writer.write_int(0x05010203, 4)
        writer.write_bytes(bytes([0x99, 0x00, 0x40, 0x30, 0x78]))
        writer.write_hex_string('C0AA0008')

        # Assert:
        expected_bytes = bytes([
            0x62, 0x61, 0x72,
            0x03, 0x02, 0x01, 0x05,
            0x99, 0x00, 0x40, 0x30, 0x78,
            0xC0, 0xAA, 0x00, 0x08
        ])
        self.assertEqual(expected_bytes, writer.buffer)

    def test_can_write_all_endianness_default(self):
        self._assert_can_write_all_endianness_little(BufferWriter())

    def test_can_write_all_endianness_little(self):
        self._assert_can_write_all_endianness_little(BufferWriter('little'))

    def test_can_write_all_endianness_big(self):
        # Arrange:
        writer = BufferWriter('big')

        # Act:
        writer.write_string('bar')
        writer.write_int(0x05010203, 4)
        writer.write_bytes(bytes([0x99, 0x00, 0x40, 0x30, 0x78]))
        writer.write_hex_string('C0AA0008')

        # Assert:
        expected_bytes = bytes([
            0x62, 0x61, 0x72,
            0x05, 0x01, 0x02, 0x03,
            0x99, 0x00, 0x40, 0x30, 0x78,
            0xC0, 0xAA, 0x00, 0x08
        ])
        self.assertEqual(expected_bytes, writer.buffer)
