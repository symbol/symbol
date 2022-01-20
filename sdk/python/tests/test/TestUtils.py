import random


class TestUtils:
	"""Common test utilities."""

	@staticmethod
	def randbytes(count):
		"""Generates random bytes. Polyfill for random.randbytes in python 3.9."""
		if hasattr(random, 'randbytes'):
			# pylint: disable=no-member
			return random.randbytes(count)

		return bytes([random.randint(0x00, 0xFF) for _ in range(0, count)])

	@staticmethod
	def random_byte_array(vector_type_class):
		"""Generates a random vector."""
		return vector_type_class(TestUtils.randbytes(vector_type_class.SIZE))
