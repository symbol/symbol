from abc import abstractmethod

from symbolchain.core.CryptoTypes import PrivateKey, PublicKey, Signature

from .NemTestUtils import NemTestUtils


class KeyPairTestDescriptor:
    def __init__(self, key_pair_class, verifier_class, deterministic_private_key, expected_public_key):
        self.key_pair_class = key_pair_class
        self.verifier_class = verifier_class
        self.deterministic_private_key = deterministic_private_key
        self.expected_public_key = expected_public_key


class BasicKeyPairTest:
    # pylint: disable=no-member

    def test_can_create_key_pair_from_private_key(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        public_key = test_descriptor.expected_public_key
        private_key = test_descriptor.deterministic_private_key

        # Act:
        key_pair = test_descriptor.key_pair_class(private_key)

        # Assert:
        self.assertEqual(public_key, key_pair.public_key)
        self.assertEqual(private_key, key_pair.private_key)

    # region sign

    def test_sign_fills_the_signature(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        key_pair = test_descriptor.key_pair_class(PrivateKey.random())
        message = NemTestUtils.randbytes(21)

        # Act:
        signature = key_pair.sign(message)

        # Assert:
        self.assertNotEqual(bytes(), signature.bytes)

    def test_signatures_generated_for_same_data_by_same_key_pairs_are_equal(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        private_key = PrivateKey.random()
        key_pair1 = test_descriptor.key_pair_class(private_key)
        key_pair2 = test_descriptor.key_pair_class(private_key)
        message = NemTestUtils.randbytes(21)

        # Act:
        signature1 = key_pair1.sign(message)
        signature2 = key_pair2.sign(message)

        # Assert:
        self.assertEqual(signature1, signature2)

    def test_signatures_generated_for_same_data_by_different_key_pairs_are_different(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        key_pair1 = test_descriptor.key_pair_class(PrivateKey.random())
        key_pair2 = test_descriptor.key_pair_class(PrivateKey.random())
        message = NemTestUtils.randbytes(21)

        # Act:
        signature1 = key_pair1.sign(message)
        signature2 = key_pair2.sign(message)

        # Assert:
        self.assertNotEqual(signature1, signature2)

    # endregion

    # region verify

    def test_signature_can_be_verified(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        message = NemTestUtils.randbytes(21)
        key_pair = test_descriptor.key_pair_class(PrivateKey.random())
        signature = key_pair.sign(message)

        # Act:
        is_verified = test_descriptor.verifier_class(key_pair.public_key).verify(message, signature)

        # Assert:
        self.assertTrue(is_verified)

    def test_signature_cannot_be_verified_with_different_key_pair(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        message = NemTestUtils.randbytes(21)
        signature = test_descriptor.key_pair_class(PrivateKey.random()).sign(message)

        # Act:
        is_verified = test_descriptor.verifier_class(NemTestUtils.randcryptotype(PublicKey)).verify(message, signature)

        # Assert:
        self.assertFalse(is_verified)

    def test_signature_does_not_verify_when_message_modified(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        message = NemTestUtils.randbytes(21)
        key_pair = test_descriptor.key_pair_class(PrivateKey.random())
        signature = key_pair.sign(message)

        verifier = test_descriptor.verifier_class(key_pair.public_key)
        for i in range(0, 21):
            mutable_message = bytearray(message)
            mutable_message[i] ^= 0xFF

            # Act:
            is_verified = verifier.verify(bytes(mutable_message), signature)

            # Assert:
            self.assertFalse(is_verified)

    def test_signature_does_not_verify_when_signature_modified(self):
        # Arrange:
        test_descriptor = self.get_test_descriptor()

        message = NemTestUtils.randbytes(21)
        key_pair = test_descriptor.key_pair_class(PrivateKey.random())
        signature = key_pair.sign(message)

        verifier = test_descriptor.verifier_class(key_pair.public_key)
        for i in range(0, Signature.SIZE):
            mutable_signature = bytearray(signature.bytes)
            mutable_signature[i] ^= 0xFF

            # Act:
            is_verified = verifier.verify(message, Signature(bytes(mutable_signature)))

            # Assert:
            self.assertFalse(is_verified)

    # endregion

    @abstractmethod
    def get_test_descriptor(self):
        pass
