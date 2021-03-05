import unittest

from symbolchain.core.CryptoTypes import PrivateKey, PublicKey, Signature
from symbolchain.tests.test.NemTestUtils import NemTestUtils


class CryptoTypesTest(unittest.TestCase):
    def test_can_create_private_key_with_correct_number_of_bytes(self):
        self._assert_can_create_byte_array_with_correct_number_of_bytes(PrivateKey, 32)

    def test_cannot_create_private_key_with_incorrect_number_of_bytes(self):
        self._assert_cannot_create_byte_array_with_incorrect_number_of_bytes(PrivateKey, 32)

    def test_can_create_random_private_key(self):
        # Act:
        private_key1 = PrivateKey.random()
        private_key2 = PrivateKey.random()

        # Assert:
        self.assertNotEqual(private_key1, private_key2)

    def test_can_create_public_key_with_correct_number_of_bytes(self):
        self._assert_can_create_byte_array_with_correct_number_of_bytes(PublicKey, 32)

    def test_cannot_create_public_key_with_incorrect_number_of_bytes(self):
        self._assert_cannot_create_byte_array_with_incorrect_number_of_bytes(PublicKey, 32)

    def test_can_create_signature_with_correct_number_of_bytes(self):
        self._assert_can_create_byte_array_with_correct_number_of_bytes(Signature, 64)

    def test_cannot_create_signature_with_incorrect_number_of_bytes(self):
        self._assert_cannot_create_byte_array_with_incorrect_number_of_bytes(Signature, 64)

    def test_equality_is_only_possible_for_same_types(self):
        # Arrange:
        raw_bytes = NemTestUtils.randbytes(32)

        # Act + Assert:
        self.assertEqual(PrivateKey(raw_bytes), PrivateKey(raw_bytes))
        self.assertEqual(PublicKey(raw_bytes), PublicKey(raw_bytes))

        self.assertNotEqual(PrivateKey(raw_bytes), PublicKey(raw_bytes))
        self.assertNotEqual(PublicKey(raw_bytes), PrivateKey(raw_bytes))

    def _assert_can_create_byte_array_with_correct_number_of_bytes(self, byte_array_type, size):
        # Arrange:
        raw_bytes = NemTestUtils.randbytes(size)

        # Act:
        byte_array = byte_array_type(raw_bytes)

        # Assert:
        self.assertEqual(raw_bytes, byte_array.bytes)

    def _assert_cannot_create_byte_array_with_incorrect_number_of_bytes(self, byte_array_type, required_size):
        for size in [0, required_size - 1, required_size + 1]:
            with self.assertRaises(ValueError):
                byte_array_type(NemTestUtils.randbytes(size))
