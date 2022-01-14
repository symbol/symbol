from binascii import hexlify, unhexlify

from .Ordered import Ordered


class ByteArray(Ordered):
	"""Represents a fixed size byte array."""

	def __init__(self, fixed_size, array_input, tag=None):
		"""Creates a byte array from bytes or hex string."""
		raw_bytes = array_input
		if isinstance(raw_bytes, str):
			raw_bytes = unhexlify(raw_bytes)

		if fixed_size != len(raw_bytes):
			raise ValueError(f'bytes was size {len(raw_bytes)} but must be {fixed_size}')

		self.bytes = raw_bytes
		self.__tag = tag

	def _cmp(self, other, operation):
		if not isinstance(other, ByteArray):
			return NotImplemented

		# pylint: disable=protected-access
		return operation(self.bytes, other.bytes) and self.__tag == other.__tag

	def __eq__(self, other):
		# pylint: disable=protected-access
		return isinstance(other, ByteArray) and self.bytes == other.bytes and self.__tag == other.__tag

	def __ne__(self, other):
		return not self == other

	def __hash__(self):
		return hash(self.bytes)

	def __str__(self):
		return hexlify(self.bytes).decode('utf8').upper()
