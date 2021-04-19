import os.path
import tempfile
import unittest

from symbolchain.core.CryptoTypes import PrivateKey
from symbolchain.core.PrivateKeyStorage import PrivateKeyStorage


class PrivateKeyStorageTest(unittest.TestCase):
    # region save

    def _assert_can_save_private_keys(self, private_key_names, password):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = PrivateKeyStorage(temp_directory, password)

            # Act:
            for name in private_key_names:
                storage.save(name, PrivateKey.random())

            # Assert:
            self.assertEqual(len(private_key_names), len(os.listdir(temp_directory)))
            for name in private_key_names:
                self.assertTrue(os.path.exists(os.path.join(temp_directory, '{}.pem'.format(name))))

    def test_can_save_single_private_key_without_encryption(self):
        self._assert_can_save_private_keys(['alpha'], None)

    def test_can_save_single_private_key_with_encryption(self):
        self._assert_can_save_private_keys(['alpha'], 'password')

    def test_can_save_multiple_private_keys_without_encryption(self):
        self._assert_can_save_private_keys(['alpha', 'beta', 'gamma'], None)

    def test_can_save_multiple_private_keys_with_encryption(self):
        self._assert_can_save_private_keys(['alpha', 'beta', 'gamma'], 'password')

    # endregion

    # region load

    def _assert_can_roundtrip_private_keys(self, private_key_names, password):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = PrivateKeyStorage(temp_directory, password)

            private_keys = [PrivateKey.random() for _ in private_key_names]

            # Act:
            for i, private_key in enumerate(private_keys):
                storage.save(private_key_names[i], private_key)

            loaded_private_keys = [storage.load(name) for name in private_key_names]

            # Assert:
            self.assertEqual(len(private_key_names), len(os.listdir(temp_directory)))
            for i, private_key in enumerate(private_keys):
                self.assertTrue(private_key, loaded_private_keys[i])

    def test_can_roundtrip_single_private_key_without_encryption(self):
        self._assert_can_roundtrip_private_keys(['alpha'], None)

    def test_can_roundtrip_single_private_key_with_encryption(self):
        self._assert_can_roundtrip_private_keys(['alpha'], 'password')

    def test_can_roundtrip_multiple_private_keys_without_encryption(self):
        self._assert_can_roundtrip_private_keys(['alpha', 'beta', 'gamma'], None)

    def test_can_roundtrip_multiple_private_keys_with_encryption(self):
        self._assert_can_roundtrip_private_keys(['alpha', 'beta', 'gamma'], 'password')

    # endregion

    # region failure cases

    def test_cannot_load_private_key_that_does_not_exist(self):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = PrivateKeyStorage(temp_directory)
            storage.save('foo', PrivateKey.random())

            # Sanity:
            self.assertEqual(1, len(os.listdir(temp_directory)))

            # Act + Assert:
            with self.assertRaises(FileNotFoundError):
                storage.load('bar')

    def _assert_cannot_load_private_key_with_wrong_password(self, password1, password2, expected_error):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage1 = PrivateKeyStorage(temp_directory, password1)
            storage1.save('foo', PrivateKey.random())

            storage2 = PrivateKeyStorage(temp_directory, password2)

            # Act + Assert:
            with self.assertRaises(expected_error):
                storage2.load('foo')

    def test_cannot_load_unencrypted_private_key_with_password(self):
        self._assert_cannot_load_private_key_with_wrong_password(None, 'password', TypeError)

    def test_cannot_load_encrypted_private_key_without_password(self):
        self._assert_cannot_load_private_key_with_wrong_password('password', None, TypeError)

    def test_cannot_load_encrypted_private_key_with_wrong_password(self):
        self._assert_cannot_load_private_key_with_wrong_password('password', 'password2', ValueError)

    # endregion
