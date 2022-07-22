from binascii import unhexlify

import cryptography

from symbolchain.CryptoTypes import PrivateKey, PublicKey
from symbolchain.impl.CipherHelpers import decode_aes_gcm, encode_aes_gcm
from symbolchain.symbol.KeyPair import KeyPair
from symbolchain.symbol.SharedKey import SharedKey

DELEGATION_MARKER = unhexlify('FE2A8061577301E2')


class MessageEncoder:
	"""Encrypts and encodes messages between two parties."""

	def __init__(self, key_pair: KeyPair):
		"""Creates message encoder around key pair."""
		self.key_pair = key_pair

	def try_decode(self, recipient_public_key, encoded_message):
		"""Tries to decode encoded message, returns tuple:
		* True, message - if message has been decoded and decrypted
		* False, encoded_message - otherwise
		"""
		if 1 == encoded_message[0]:
			try:
				message = decode_aes_gcm(SharedKey, self.key_pair, recipient_public_key, encoded_message[1:])
				return True, message
			except cryptography.exceptions.InvalidTag:
				pass
		elif 0xFE == encoded_message[0] and DELEGATION_MARKER == encoded_message[:8]:
			try:
				ephemeral_public_key = PublicKey(encoded_message[len(DELEGATION_MARKER):len(DELEGATION_MARKER) + PublicKey.SIZE])
				message = decode_aes_gcm(SharedKey, self.key_pair, ephemeral_public_key, encoded_message[len(DELEGATION_MARKER) + PublicKey.SIZE:])
				return True, message
			except cryptography.exceptions.InvalidTag:
				pass
			except ValueError as exception:
				exceptions = ['point is not in main subgroup']
				if not any(map(lambda message: message in str(exception), exceptions)):
					raise

		return False, encoded_message

	@staticmethod
	def encode_persistent_harvesting_delegation(node_public_key, remote_key_pair, vrf_root_key_pair):  # pylint: disable=invalid-name
		"""Encodes persistent harvesting delegation to node."""
		ephemeral_key_pair = KeyPair(PrivateKey.random())

		message = remote_key_pair.private_key.bytes + vrf_root_key_pair.private_key.bytes
		tag, initialization_vector, cipher_text = encode_aes_gcm(SharedKey, ephemeral_key_pair, node_public_key, message)

		return DELEGATION_MARKER + ephemeral_key_pair.public_key.bytes + tag + initialization_vector + cipher_text

	def encode(self, recipient_public_key: PublicKey, message: bytes):
		"""Encodes message to recipient using recommended format."""

		tag, initialization_vector, cipher_text = encode_aes_gcm(SharedKey, self.key_pair, recipient_public_key, message)
		return b'\1' + tag + initialization_vector + cipher_text
