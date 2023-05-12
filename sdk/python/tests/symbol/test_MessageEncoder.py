import unittest
import warnings

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.symbol.KeyPair import KeyPair
from symbolchain.symbol.MessageEncoder import MessageEncoder

from ..test.BasicMessageEncoderTest import BasicMessageEncoderTest, MessageEncoderDecodeFailureTest, MessageEncoderTestInterface


def malform_message(encoded_bytes):
	return encoded_bytes[:-1] + bytes(encoded_bytes[-1] ^ 0xFF)


class MessageEncoderTests(BasicMessageEncoderTest, unittest.TestCase):
	def get_basic_test_interface(self):
		return MessageEncoderTestInterface(KeyPair, MessageEncoder, None, None, malform_message)

	def test_decode_falls_back_to_input_when_message_has_unknown_type(self):
		# Arrange:
		encoder = MessageEncoder(KeyPair(PrivateKey.random()))

		# Act:
		result, message = encoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), b'\2hello')

		# Assert:
		self.assertFalse(result)
		self.assertEqual(b'\2hello', message)


class MessageEncoderDelegationTests(MessageEncoderDecodeFailureTest, unittest.TestCase):
	def get_basic_test_interface(self):
		def encode_persistent_harvesting_delegation(encoder):   # pylint: disable=invalid-name
			def encode(_1, _2):
				# simulate a delegation message where node and ephemeral key pairs are used
				# for these tests to work properly, the encoder key pair is used as the node key pair
				remote_key_pair = KeyPair(PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'))
				vrf_key_pair = KeyPair(PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'))
				return encoder.encode_persistent_harvesting_delegation(encoder.key_pair.public_key, remote_key_pair, vrf_key_pair)

			return encode

		# notice that only failure tests are being run via this interface
		return MessageEncoderTestInterface(KeyPair, MessageEncoder, encode_persistent_harvesting_delegation, None, malform_message)

	# note: there's no sender decode test for persistent harvesting delegation, because sender does not have ephemeral key pair

	def test_recipient_can_decode_encoded_persistent_harvesting_delegation(self):
		# Arrange:
		key_pair = KeyPair(PrivateKey.random())
		node_key_pair = KeyPair(PrivateKey.random())
		remote_key_pair = KeyPair(PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'))
		vrf_key_pair = KeyPair(PrivateKey('11223344556677889900AABBCCDDEEFF11223344556677889900AABBCCDDEEFF'))
		encoder = MessageEncoder(key_pair)
		encoded = encoder.encode_persistent_harvesting_delegation(node_key_pair.public_key, remote_key_pair, vrf_key_pair)

		# Act:
		decoder = MessageEncoder(node_key_pair)
		result, decoded = decoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), encoded)

		# Assert:
		self.assertTrue(result)
		self.assertEqual(remote_key_pair.private_key.bytes + vrf_key_pair.private_key.bytes, decoded)

	def test_decode_falls_back_to_input_when_ephemeral_public_key_is_not_valid(self):
		# Arrange:
		key_pair = KeyPair(PrivateKey.random())
		node_key_pair = KeyPair(PrivateKey.random())
		remote_key_pair = KeyPair(PrivateKey.random())
		vrf_key_pair = KeyPair(PrivateKey.random())
		encoder = MessageEncoder(key_pair)
		encoded = encoder.encode_persistent_harvesting_delegation(node_key_pair.public_key, remote_key_pair, vrf_key_pair)

		encoded = encoded[0:8] + bytes(32) + encoded[8 + 32:]

		# Act:
		decoder = MessageEncoder(node_key_pair)
		result, decoded = decoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), encoded)

		# Assert:
		self.assertFalse(result)
		self.assertEqual(encoded, decoded)


class MessageEncoderDeprecatedTests(BasicMessageEncoderTest, unittest.TestCase):
	@staticmethod
	def try_decode_deprecated(decoder):
		def try_decode(recipient_public_key, encoded_message):
			with warnings.catch_warnings():
				warnings.simplefilter('ignore')

				return decoder.try_decode_deprecated(recipient_public_key, encoded_message)

		return try_decode

	def get_basic_test_interface(self):
		def encode_deprecated(encoder):
			def encode(recipient_public_key, message):
				with warnings.catch_warnings():
					warnings.simplefilter('ignore')

					return encoder.encode_deprecated(recipient_public_key, message)

			return encode

		return MessageEncoderTestInterface(KeyPair, MessageEncoder, encode_deprecated, self.try_decode_deprecated, malform_message)

	def test_decode_deprecated_falls_back_to_decode_on_failure(self):
		# Arrange: encode using non-deprecated function
		key_pair = KeyPair(PrivateKey.random())
		recipient_public_key = KeyPair(PrivateKey.random()).public_key
		encoder = MessageEncoder(key_pair)
		encoded = encoder.encode(recipient_public_key, b'hello world')

		# Act: decode using deprecated function
		result, decoded = self.try_decode_deprecated(encoder)(recipient_public_key, encoded)

		# Assert: decode was successful
		self.assertTrue(result)
		self.assertEqual(b'hello world', decoded)
