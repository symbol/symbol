import hashlib


def mosaic_restriction_generate_key(seed):
	"""Generates a mosaic restriction key from a string."""

	hash_result = hashlib.sha3_256(seed.encode('utf8')).digest()
	key_bytes = bytearray(hash_result[:8])

	return int.from_bytes(key_bytes, 'little')
