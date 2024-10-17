import sha3
from nacl.bindings import (
	crypto_core_ed25519_is_valid_point,
	crypto_core_ed25519_scalar_add,
	crypto_core_ed25519_scalar_mul,
	crypto_core_ed25519_scalar_reduce,
	crypto_core_ed25519_sub,
	crypto_scalarmult_ed25519_base,
	crypto_scalarmult_ed25519_base_noclamp,
	crypto_scalarmult_ed25519_noclamp
)

from ..CryptoTypes import PrivateKey, PublicKey, Signature


def _generate_nonce(secret_key, message):
	hashed_secret = sha3.keccak_512(secret_key).digest()
	hashobj = sha3.keccak_512()
	hashobj.update(hashed_secret[32:])
	hashobj.update(message)
	final_hash = hashobj.digest()
	return final_hash


def _is_reduced_s(encoded_s):
	reduced = crypto_core_ed25519_scalar_reduce(encoded_s + bytes(32))
	return reduced == encoded_s


def _is_canonical_s(encoded_s):
	# reject zero S
	if encoded_s == bytes(32):
		return False

	# reject non-reduced S
	return _is_reduced_s(encoded_s)


class KeyPair:
	"""Represents an ED25519 private and public key."""

	def __init__(self, private_key):
		"""Creates a key pair from a private key."""
		self._sk = private_key.bytes[::-1]

		# nacl does clamping
		hashed_secret = sha3.keccak_512(self._sk).digest()
		self._pk = crypto_scalarmult_ed25519_base(hashed_secret[:32])

	@property
	def public_key(self):
		"""Gets the public key."""
		return PublicKey(self._pk)

	@property
	def private_key(self):
		"""Gets the private key."""
		return PrivateKey(self._sk[::-1])

	def sign(self, message):
		"""Signs a message with the private key."""

		# pylint: disable=invalid-name
		# r = H(privHash[256:512] || data)
		r = _generate_nonce(self._sk, message)

		# R = r * base point
		r = crypto_core_ed25519_scalar_reduce(r)
		R = crypto_scalarmult_ed25519_base_noclamp(r)

		# h = H(encodedR || public || data)
		hashobj = sha3.keccak_512()
		hashobj.update(R[:32])
		hashobj.update(self._pk)
		hashobj.update(message)
		h = hashobj.digest()
		h = crypto_core_ed25519_scalar_reduce(h)

		# hash and clamp private key
		a = bytearray(sha3.keccak_512(self._sk).digest()[:32])
		a[0] &= 0xF8
		a[31] &= 0x7F
		a[31] |= 0x40
		a = bytes(a)

		# S = (r + h * a) mod L
		h_a = crypto_core_ed25519_scalar_mul(a, h)
		S = crypto_core_ed25519_scalar_add(r, h_a)

		# ensure resulting signature is canonical (this is a sanity check that should never get triggered)
		assert bytes(32) == S or _is_reduced_s(S)

		act_sig = R + S
		return Signature(act_sig)


class Verifier:
	"""Verifies signatures signed by a single key pair."""

	def __init__(self, public_key):
		"""Creates a verifier from a public key."""
		if bytes(32) == public_key.bytes:
			raise ValueError('public key cannot be zero')

		self._pk = public_key.bytes

	def verify(self, message, signature):
		"""Verifies a message signature."""

		encoded_r = signature.bytes[:32]
		encoded_s = signature.bytes[32:]

		if not _is_canonical_s(encoded_s):
			return False

		# h = H(encodedR || public || data)
		hashobj = sha3.keccak_512()
		hashobj.update(encoded_r)
		hashobj.update(self._pk)
		hashobj.update(message)
		h = crypto_core_ed25519_scalar_reduce(hashobj.digest())  # pylint: disable=invalid-name

		# note: this check covers following:
		#  * (y) coord is canonical (< 2^255 - 19)
		#  * has small order - this is slightly stronger check,
		#    we don't do that in the client as we verify sigs with pub keys that are known to belong to accounts
		#  * point is on curve
		#  * point is in main subgroup
		if not crypto_core_ed25519_is_valid_point(self._pk):
			return False

		# R = encodedS * B - h * A
		s_b = crypto_scalarmult_ed25519_base_noclamp(encoded_s)
		h_a = crypto_scalarmult_ed25519_noclamp(h, self._pk)

		computed_r = crypto_core_ed25519_sub(s_b, h_a)
		return computed_r == encoded_r
