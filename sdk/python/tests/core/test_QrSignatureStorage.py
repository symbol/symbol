import tempfile
import unittest

from symbolchain.core.CryptoTypes import Hash256, Signature
from symbolchain.core.QrSignatureStorage import QrSignatureStorage

from ..test.NemTestUtils import NemTestUtils


def write_random_qrcode(directory, name, data_size):
    # abuse transaction_hash by changing its underlying bytes
    transaction_hash = NemTestUtils.randcryptotype(Hash256)
    transaction_hash.bytes = NemTestUtils.randbytes(data_size)
    QrSignatureStorage(directory).save(name, transaction_hash, [])


class QrSignatureStorageTest(unittest.TestCase):
    def _assert_can_roundtrip_signatures(self, num_signatures):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            storage = QrSignatureStorage(temp_directory)

            transaction_hash = NemTestUtils.randcryptotype(Hash256)
            signatures = [NemTestUtils.randcryptotype(Signature) for _ in range(0, num_signatures)]
            storage.save('foo', transaction_hash, signatures)

            # Act:
            (loaded_transaction_hash, loaded_signatures) = storage.load('foo')

            # Assert:
            self.assertEqual(transaction_hash, loaded_transaction_hash)
            self.assertEqual(signatures, loaded_signatures)

    def test_can_roundtrip_zero_signatures(self):
        self._assert_can_roundtrip_signatures(0)

    def test_can_roundtrip_single_signature(self):
        self._assert_can_roundtrip_signatures(1)

    def test_can_roundtrip_multiple_signatures(self):
        self._assert_can_roundtrip_signatures(5)

    def _assert_cannot_load_qrcode(self, data_size):
        # Arrange:
        with tempfile.TemporaryDirectory() as temp_directory:
            write_random_qrcode(temp_directory, 'foo', data_size)
            storage = QrSignatureStorage(temp_directory)

            with self.assertRaises(ValueError):
                storage.load('foo')

    def test_cannot_load_qrcode_containing_insufficient_data(self):
        self._assert_cannot_load_qrcode(Hash256.SIZE - 1)  # less than transaction_hash

    def test_cannot_load_qrcode_containing_partial_signature(self):
        self._assert_cannot_load_qrcode(Hash256.SIZE + Signature.SIZE * 2 + Signature.SIZE // 2)  # 2.5 signatures
