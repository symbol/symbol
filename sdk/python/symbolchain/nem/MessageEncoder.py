import cryptography

from symbolchain.CryptoTypes import PublicKey
from symbolchain.impl.CipherHelpers import decode_aes_cbc, decode_aes_gcm, encode_aes_cbc, encode_aes_gcm
from symbolchain.nc import Message, MessageType
from symbolchain.nem.KeyPair import KeyPair
from symbolchain.nem.SharedKey import SharedKey


class MessageEncoder:
	"""Encrypts and encodes messages between two parties."""

	def __init__(self, key_pair: KeyPair):
		"""Creates message encoder around key pair."""
		self.key_pair = key_pair

	def try_decode(self, recipient_public_key, encoded_message: Message):
		"""Tries to decode encoded message, returns tuple:
		* True, message - if message has been decoded and decrypted
		* False, encoded_message - otherwise
		"""

		if MessageType.ENCRYPTED != encoded_message.message_type:
			raise RuntimeError('invalid message format')

		try:
			message = decode_aes_gcm(SharedKey, self.key_pair, recipient_public_key, encoded_message.message)
			return True, message
		except cryptography.exceptions.InvalidTag:
			pass

		try:
			message = decode_aes_cbc(SharedKey, self.key_pair, recipient_public_key, encoded_message.message)
			return True, message
		except ValueError as exception:
			exceptions = [
				'Invalid padding bytes',
				'Invalid IV size',
				'The length of the provided data is not a multiple of the block length']
			if not any(map(lambda message: message in str(exception), exceptions)):
				raise

		return False, encoded_message

	def encode_deprecated(self, recipient_public_key: PublicKey, message: bytes):
		"""Encodes message to recipient using deprecated encryption and key derivation."""

		salt, initialization_vector, cipher_text = encode_aes_cbc(SharedKey, self.key_pair, recipient_public_key, message)

		encoded_messsage = Message()
		encoded_messsage.message_type = MessageType.ENCRYPTED
		encoded_messsage.message = salt + initialization_vector + cipher_text
		return encoded_messsage

	def encode(self, recipient_public_key: PublicKey, message: bytes):
		"""Encodes message to recipient using recommended format."""

		tag, initialization_vector, cipher_text = encode_aes_gcm(SharedKey, self.key_pair, recipient_public_key, message)

		encoded_messsage = Message()
		encoded_messsage.message_type = MessageType.ENCRYPTED
		encoded_messsage.message = tag + initialization_vector + cipher_text
		return encoded_messsage
