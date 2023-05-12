import unittest
import warnings

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.nc import Message, MessageType
from symbolchain.nem.KeyPair import KeyPair
from symbolchain.nem.MessageEncoder import MessageEncoder

from ..test.BasicMessageEncoderTest import BasicMessageEncoderTest, MessageEncoderTestInterface


def malform_message(encoded):
	crafted_byte = encoded.message[-1] ^ 0xFF
	encoded.message = encoded.message[:-1] + bytes([crafted_byte])
	return encoded


class MessageEncoderTests(BasicMessageEncoderTest, unittest.TestCase):
	def get_basic_test_interface(self):
		return MessageEncoderTestInterface(KeyPair, MessageEncoder, None, None, malform_message)

	def test_decode_falls_back_to_input_when_cbc_block_size_is_invalid(self):
		# Arrange:
		encoder = MessageEncoder(KeyPair(PrivateKey.random()))
		recipient_public_key = KeyPair(PrivateKey.random()).public_key

		encoded_message = Message()
		encoded_message.message_type = MessageType.ENCRYPTED
		encoded_message.message = bytes(16 + 32 + 1)

		# Act:
		result, decoded = encoder.try_decode(recipient_public_key, encoded_message)

		# Assert
		self.assertFalse(result)
		self.assertEqual(decoded, encoded_message)

	def test_decode_throws_when_message_type_is_invalid(self):
		# Arrange:
		encoder = MessageEncoder(KeyPair(PrivateKey.random()))
		encoded_message = Message()
		encoded_message.message_type = MessageType.PLAIN

		# Act + Assert:
		with self.assertRaises(RuntimeError):
			encoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), encoded_message)


class MessageEncoderDeprecatedTests(BasicMessageEncoderTest, unittest.TestCase):
	def get_basic_test_interface(self):
		def encode_deprecated(encoder):
			def encode(recipient_public_key, message):
				with warnings.catch_warnings():
					warnings.simplefilter('ignore')

					return encoder.encode_deprecated(recipient_public_key, message)

			return encode

		return MessageEncoderTestInterface(KeyPair, MessageEncoder, encode_deprecated, None, malform_message)
