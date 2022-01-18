from enum import Enum


class DisplayType(Enum):
	"""Enumeration of AST object display types."""

	UNSET = 0
	INTEGER = 1
	BYTE_ARRAY = 2
	TYPED_ARRAY = 3
	ENUM = 4
	STRUCT = 5

	@property
	def is_array(self):
		"""Returns true if this display type is an array type."""
		return self in (DisplayType.BYTE_ARRAY, DisplayType.TYPED_ARRAY)
