import hashlib

if 'ripemd160' not in hashlib.algorithms_guaranteed:
	from ripemd import ripemd160 as ripemd160_impl


def _factory():
	return hashlib.new('ripemd160') if 'ripemd160' in hashlib.algorithms_guaranteed else ripemd160_impl.new()


def ripemd160(data):
	"""Calculates RIPEMD-160 hash of data."""

	builder = _factory()
	builder.update(data)
	return builder.digest()
