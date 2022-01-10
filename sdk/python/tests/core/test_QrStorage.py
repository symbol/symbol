import os
import tempfile
import unittest

import qrcode

from symbolchain.core.QrStorage import QrStorage

from ..test.NemTestUtils import NemTestUtils

MAX_DATA_BYTES = 1155


def write_random_qrcode(directory, name, data_size):
    QrStorage(directory).save(name, NemTestUtils.randbytes(data_size))


class QrStorageTest(unittest.TestCase):
    # region save

    def _assert_can_save_buffer(self, data_size):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = QrStorage(temp_directory)

            buffer = NemTestUtils.randbytes(data_size)

            # Act:
            storage.save('foo', buffer)

            # Assert:
            self.assertEqual(1, len(os.listdir(temp_directory)))
            self.assertTrue(os.path.exists(os.path.join(temp_directory, 'foo.png')))

    def test_can_save_empty_buffer(self):
        self._assert_can_save_buffer(0)

    def test_can_save_nonempty_buffer(self):
        self._assert_can_save_buffer(100)

    def test_can_save_max_buffer(self):
        self._assert_can_save_buffer(MAX_DATA_BYTES)

    def test_cannot_save_more_than_max_buffer(self):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = QrStorage(temp_directory)

            buffer = NemTestUtils.randbytes(MAX_DATA_BYTES + 1)

            # Act + Assert:
            with self.assertRaises(qrcode.exceptions.DataOverflowError):
                storage.save('foo', buffer)

    # endregion

    # region load

    def _assert_can_roundtrip_buffer(self, data_size):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = QrStorage(temp_directory)

            buffer = NemTestUtils.randbytes(data_size)
            storage.save('foo', buffer)

            # Act:
            loaded_buffer = storage.load('foo')

            # Assert:
            self.assertEqual(buffer, loaded_buffer)

    def test_can_roundtrip_empty_buffer(self):
        self._assert_can_roundtrip_buffer(0)

    def test_can_roundtrip_nonempty_buffer(self):
        self._assert_can_roundtrip_buffer(100)

    def test_cannot_load_qrcode_that_does_not_exist(self):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = QrStorage(temp_directory)

            buffer = NemTestUtils.randbytes(100)
            storage.save('foo', buffer)

            # Sanity:
            self.assertEqual(1, len(os.listdir(temp_directory)))

            # Act + Assert:
            with self.assertRaises(FileNotFoundError):
                storage.load('bar')

    # endregion
