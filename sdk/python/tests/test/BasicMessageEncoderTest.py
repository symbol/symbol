from abc import abstractmethod
from collections import namedtuple

from symbolchain.CryptoTypes import PrivateKey

MessageEncoderTestInterface = namedtuple('MessageEncoderTestInterface', [
	'key_pair_class', 'encoder_class', 'encode', 'try_decode', 'malform'
])


def _encode(interface, encoder):
	return interface.encode(encoder) if interface.encode else encoder.encode


def _try_decode(interface, decoder):
	return interface.try_decode(decoder) if interface.try_decode else decoder.try_decode


class MessageEncoderDecodeFailureTest:
	# pylint: disable=no-member
	def _assert_decode_falls_back_to_input_on_failure(self, message):
		# Arrange:
		interface = self.get_basic_test_interface()
		key_pair = interface.key_pair_class(PrivateKey.random())
		recipient_public_key = interface.key_pair_class(PrivateKey.random()).public_key
		encoder = interface.encoder_class(key_pair)
		encoded = _encode(interface, encoder)(recipient_public_key, message)

		encoded = interface.malform(encoded)

		# Act:
		result, decoded = _try_decode(interface, encoder)(recipient_public_key, encoded)

		# Assert:
		self.assertFalse(result)
		self.assertEqual(decoded, encoded)

	def test_decode_falls_back_to_input_when_decoding_failed_short(self):
		self._assert_decode_falls_back_to_input_on_failure(b'hello world')

	def test_decode_falls_back_to_input_when_decoding_failed_long(self):
		self._assert_decode_falls_back_to_input_on_failure(b'bit longer message that should span upon multiple encryption blocks')

	@abstractmethod
	def get_basic_test_interface(self):
		pass


class BasicMessageEncoderTest(MessageEncoderDecodeFailureTest):
	# pylint: disable=no-member

	def test_sender_can_decode_encoded_message(self):
		# Arrange:
		interface = self.get_basic_test_interface()
		key_pair = interface.key_pair_class(PrivateKey.random())
		recipient_public_key = interface.key_pair_class(PrivateKey.random()).public_key
		encoder = interface.encoder_class(key_pair)
		encoded = _encode(interface, encoder)(recipient_public_key, b'hello world')

		# Act:
		result, decoded = _try_decode(interface, encoder)(recipient_public_key, encoded)

		# Assert:
		self.assertTrue(result)
		self.assertEqual(decoded, b'hello world')

	def test_recipient_can_decode_encoded_message(self):
		# Arrange:
		interface = self.get_basic_test_interface()
		key_pair = interface.key_pair_class(PrivateKey.random())
		recipient_key_pair = interface.key_pair_class(PrivateKey.random())
		encoder = interface.encoder_class(key_pair)
		encoded = _encode(interface, encoder)(recipient_key_pair.public_key, b'hello world')

		# Act:
		decoder = interface.encoder_class(recipient_key_pair)
		result, decoded = _try_decode(interface, decoder)(key_pair.public_key, encoded)

		# Assert:
		self.assertTrue(result)
		self.assertEqual(decoded, b'hello world')

	@abstractmethod
	def get_basic_test_interface(self):
		pass
