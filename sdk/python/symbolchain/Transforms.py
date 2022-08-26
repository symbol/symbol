import sha3

from symbolchain.ripemd160 import ripemd160


def ripemd_keccak_256(payload):
	"""Hashes payload with keccak 256 and then hashes the result with ripemd160."""

	return ripemd160(sha3.keccak_256(payload).digest())
