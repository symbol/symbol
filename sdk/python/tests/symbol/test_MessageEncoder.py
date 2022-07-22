from binascii import unhexlify
import unittest

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.symbol.KeyPair import KeyPair
from symbolchain.symbol.MessageEncoder import MessageEncoder

from ..test.BasicMessageEncoderTest import BasicMessageEncoderTest, MessageEncoderDecodeFailureTest, MessageEncoderTestInterface


def fake_delegation(encoder):
	def fake_delegation_encoder(public_key, message):
		encoded = encoder.encode(public_key, message)
		return unhexlify('FE2A8061577301E2') + public_key.bytes + encoded[1:]

	return fake_delegation_encoder

class MessageEncoderFakeDelegationTests(MessageEncoderDecodeFailureTest, unittest.TestCase):
	def get_basic_test_interface(self):
		return MessageEncoderTestInterface(
			KeyPair,
			MessageEncoder,
			fake_delegation,
			lambda encoded: encoded[:-1] + bytes([encoded[-1] ^ 0xFF]))


class MessageEncoderTests(BasicMessageEncoderTest, unittest.TestCase):
	def get_basic_test_interface(self):
		return MessageEncoderTestInterface(
			KeyPair,
			MessageEncoder,
			lambda encoder: encoder.encode,
			lambda encoded: encoded[:-1] + bytes([encoded[-1] ^ 0xFF]))

	# note: there's no sender decode test for persistent harvesting delegation, cause sender does not have ephemeral key pair

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

		encoded = encoded[0:8] + bytes(32) + encoded[8+32:]

		# Act:
		decoder = MessageEncoder(node_key_pair)
		result, decoded = decoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), encoded)

		# Assert:
		self.assertFalse(result)
		self.assertEqual(encoded, decoded)

	def test_decode_falls_back_to_input_when_message_has_unknown_type(self):
		# Arrange:
		encoder = MessageEncoder(KeyPair(PrivateKey.random()))

		# Act:
		result, message = encoder.try_decode(PublicKey(bytes(PublicKey.SIZE)), b'\2hello')

		# Assert:
		self.assertFalse(result)
		self.assertEqual(b'\2hello', message)
