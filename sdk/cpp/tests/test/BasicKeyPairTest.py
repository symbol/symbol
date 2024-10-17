from abc import abstractmethod
from binascii import unhexlify

from symbolchain.CryptoTypes import PrivateKey, PublicKey, Signature

from .TestUtils import TestUtils


class KeyPairTestDescriptor:
	def __init__(self, key_pair_class, verifier_class, deterministic_private_key, expected_public_key):
		self.key_pair_class = key_pair_class
		self.verifier_class = verifier_class
		self.deterministic_private_key = deterministic_private_key
		self.expected_public_key = expected_public_key


class BasicKeyPairTest:
	# pylint: disable=no-member

	# region create

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

	# endregion

	# region sign

	def test_sign_fills_the_signature(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		key_pair = test_descriptor.key_pair_class(PrivateKey.random())
		message = TestUtils.randbytes(21)

		# Act:
		signature = key_pair.sign(message)

		# Assert:
		self.assertNotEqual(Signature.zero(), signature)

	def test_signatures_generated_for_same_data_by_same_key_pairs_are_equal(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		private_key = PrivateKey.random()
		key_pair1 = test_descriptor.key_pair_class(private_key)
		key_pair2 = test_descriptor.key_pair_class(private_key)
		message = TestUtils.randbytes(21)

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
		message = TestUtils.randbytes(21)

		# Act:
		signature1 = key_pair1.sign(message)
		signature2 = key_pair2.sign(message)

		# Assert:
		self.assertNotEqual(signature1, signature2)

	# endregion

	# region verify

	def test_cannot_create_verifier_around_zero_public_key(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		zero_public_key = PublicKey(bytes(PublicKey.SIZE))

		# Act + Assert:
		with self.assertRaises(ValueError):
			test_descriptor.verifier_class(zero_public_key)

	def test_signature_can_be_verified(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		message = TestUtils.randbytes(21)
		key_pair = test_descriptor.key_pair_class(PrivateKey.random())
		signature = key_pair.sign(message)

		# Act:
		is_verified = test_descriptor.verifier_class(key_pair.public_key).verify(message, signature)

		# Assert:
		self.assertTrue(is_verified)

	def test_signature_cannot_be_verified_with_different_key_pair(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		message = TestUtils.randbytes(21)
		signature = test_descriptor.key_pair_class(PrivateKey.random()).sign(message)

		# Act:
		is_verified = test_descriptor.verifier_class(TestUtils.random_byte_array(PublicKey)).verify(message, signature)

		# Assert:
		self.assertFalse(is_verified)

	def test_signature_does_not_verify_when_message_modified(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		message = TestUtils.randbytes(21)
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

		message = TestUtils.randbytes(21)
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

	def test_signature_with_zero_s_does_not_verify(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		message = TestUtils.randbytes(21)
		key_pair = test_descriptor.key_pair_class(PrivateKey.random())
		signature = key_pair.sign(message)
		signature_zero_s = Signature(signature.bytes[:32] + bytes(32))

		verifier = test_descriptor.verifier_class(key_pair.public_key)

		# Act:
		is_verified = verifier.verify(message, signature)
		is_verified_zero_s = verifier.verify(message, signature_zero_s)

		# Assert:
		self.assertTrue(is_verified)
		self.assertFalse(is_verified_zero_s)

	@staticmethod
	def _scalar_add_group_order(scalar):
		# 2^252 + 27742317777372353535851937790883648493, little endian
		group_order = unhexlify('EDD3F55C1A631258D69CF7A2DEF9DE1400000000000000000000000000000010')
		remainder = 0

		for i, group_order_byte in enumerate(group_order):
			byte_sum = scalar[i] + group_order_byte
			scalar[i] = (byte_sum + remainder) & 0xFF
			remainder = (byte_sum >> 8) & 0xFF

		return scalar

	def test_non_canonical_signature_does_not_verify(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		# the value 30 in the payload ensures that the encodedS part of the signature is < 2 ^ 253 after adding the group order
		message = unhexlify('0102030405060708091D')
		key_pair = test_descriptor.key_pair_class(PrivateKey.random())
		canonical_signature = key_pair.sign(message)

		non_canonical_signature = Signature(
			canonical_signature.bytes[:32] + self._scalar_add_group_order(bytearray(canonical_signature.bytes[32:]))
		)

		verifier = test_descriptor.verifier_class(key_pair.public_key)

		# Act:
		is_verified_canonical = verifier.verify(message, canonical_signature)
		is_verified_non_canonical = verifier.verify(message, non_canonical_signature)

		# Assert:
		self.assertTrue(is_verified_canonical)
		self.assertFalse(is_verified_non_canonical)

	# endregion

	@abstractmethod
	def get_test_descriptor(self):
		pass
