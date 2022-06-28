import hashlib

import sha3


def ripemd_keccak_256(payload):
	"""Hashes payload with keccak 256 and then hashes the result with ripemd160."""

	outer_hasher = hashlib.new('ripemd160')
	outer_hasher.update(sha3.keccak_256(payload).digest())
	return outer_hasher.digest()
