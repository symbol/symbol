from ..SharedKey import SharedKey as BasicSharedKey


class SharedKey(BasicSharedKey):
	@staticmethod
	def derive_shared_key(key_pair, other_public_key):
		"""Derives shared encryption key from private key and public key."""

		return BasicSharedKey._derive_shared_key(other_public_key.bytes, key_pair.private_key.bytes, b'catapult')
