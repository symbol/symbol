import sha3

from ..CryptoTypes import SharedKey256
from ..external import ed25519
from ..SharedKey import SharedKey as BasicSharedKey


class SharedKey(BasicSharedKey):
	@staticmethod
	def derive_shared_key(key_pair, other_public_key):
		"""Derives shared encryption key from private key and public key."""

		# note: key_pair.private_key is "unreversed", so need to reverse here again
		return BasicSharedKey._derive_shared_key(other_public_key.bytes, key_pair.private_key.bytes[::-1], b'nem-nis1', sha3.keccak_512)

	@staticmethod
	def derive_shared_key_deprecated(key_pair, other_public_key, salt):
		"""Derives shared encryption key from private key, public key and salt.

		note: this method uses _old_ method of deriving shared key and has been deprecated.
		"""

		# note: key_pair.private_key is "unreversed", so need to reverse here again
		shared_secret = ed25519.derive_shared_secret_unsafe(other_public_key.bytes, key_pair.private_key.bytes[::-1], sha3.keccak_512)
		key = bytes([shared_secret[i] ^ salt[i] for i in range(32)])
		return SharedKey256(sha3.keccak_256(key).digest())
