from .Ordered import Ordered


class BaseValue(Ordered):
	"""Represents a base int."""

	def __init__(self, size, value, tag=None, signed=False):
		"""Creates a base value."""
		self.size = size
		self.value = value
		self.__tag = (tag, signed)

		# check bounds
		bit_size = self.size * 8
		if signed:
			upper_bound = (1 << (bit_size - 1)) - 1
			lower_bound = -upper_bound - 1
		else:
			upper_bound = (1 << bit_size) - 1
			lower_bound = 0

		if self.value < lower_bound or self.value > upper_bound:
			signed_description = 'signed' if signed else 'unsigned'
			value_range_message = f'{value} must be in range [{lower_bound}, {upper_bound}]'
			raise ValueError(f'{value_range_message} for {self.size} bytes ({signed_description})')

	def _cmp(self, other, operation):
		if not isinstance(other, BaseValue):
			return NotImplemented

		# pylint: disable=protected-access
		return operation(self.value, other.value) and self.__tag == other.__tag

	def __eq__(self, other):
		# pylint: disable=protected-access
		return isinstance(other, BaseValue) and self.value == other.value and self.__tag == other.__tag

	def __ne__(self, other):
		return not self == other

	def __hash__(self):
		return hash((self.value, self.__tag))

	def __str__(self):
		if not self.__tag[1] or self.value >= 0:
			return f'0x{self.value:0{self.size * 2}X}'

		return f'-0x{-self.value:0{self.size * 2}X}'
