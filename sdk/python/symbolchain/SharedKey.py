import hashlib

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.hkdf import HKDF

from .CryptoTypes import SharedKey256
from .external import ed25519


class SharedKey:
	@staticmethod
	def _derive_shared_key(other_public_key_bytes, private_key_bytes, info, hashing_algorithm=hashlib.sha512):
		shared_secret = ed25519.derive_shared_secret_unsafe(other_public_key_bytes, private_key_bytes, hashing_algorithm)
		salt = bytes(32)
		hkdf = HKDF(algorithm=hashes.SHA256(), length=32, salt=salt, info=info)
		return SharedKey256(hkdf.derive(shared_secret))
