from cryptography.hazmat.primitives import padding
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

# region AesCbcCipher


class AesCbcCipher:
	"""Performs AES CBC encryption and decryption with a given key."""

	def __init__(self, aes_key):
		"""Creates a cipher around an aes shared key."""

		self.key = aes_key

	def encrypt(self, clear_text, iv):  # pylint: disable=invalid-name
		"""Encrypts clear text."""

		# input needs to be padded in CBC mode to a multiple of block size
		padder = padding.PKCS7(128).padder()
		padded_clear_text = padder.update(clear_text)
		padded_clear_text += padder.finalize()

		encryptor = Cipher(algorithms.AES(self.key.bytes), modes.CBC(iv)).encryptor()
		return encryptor.update(padded_clear_text) + encryptor.finalize()

	def decrypt(self, cipher_text, iv):  # pylint: disable=invalid-name
		"""Decrypts cipher text."""

		decryptor = Cipher(algorithms.AES(self.key.bytes), modes.CBC(iv)).decryptor()
		clear_text = decryptor.update(cipher_text) + decryptor.finalize()

		unpadder = padding.PKCS7(128).unpadder()
		unpadded_cipher_text = unpadder.update(clear_text)
		unpadded_cipher_text += unpadder.finalize()
		return unpadded_cipher_text

# endregion


# region AesGcmCipher

class AesGcmCipher:
	"""Performs AES GCM encryption and decryption with a given key."""

	TAG_SIZE = 16

	def __init__(self, aes_key):
		"""Creates a cipher around an aes shared key."""

		self.key = aes_key

	def encrypt(self, clear_text, iv):  # pylint: disable=invalid-name
		"""Encrypts clear text and appends tag to encrypted payload."""

		encryptor = Cipher(algorithms.AES(self.key.bytes), modes.GCM(iv)).encryptor()
		cipher_text = encryptor.update(clear_text) + encryptor.finalize()
		return cipher_text + encryptor.tag

	def decrypt(self, cipher_text, iv):  # pylint: disable=invalid-name
		"""Decrypts cipher text with appended tag."""

		tag_start_offset = len(cipher_text) - self.TAG_SIZE
		decryptor = Cipher(algorithms.AES(self.key.bytes), modes.GCM(iv, cipher_text[tag_start_offset:])).decryptor()
		return decryptor.update(cipher_text[:tag_start_offset]) + decryptor.finalize()

# endregion
