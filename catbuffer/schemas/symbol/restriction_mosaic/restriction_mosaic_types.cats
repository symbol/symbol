using MosaicRestrictionKey = uint64

# Enumeration of mosaic restriction types.
enum MosaicRestrictionType : uint8
	# Uninitialized value indicating no restriction.
	NONE = 0x00

	# Allow if equal.
	EQ = 0x01

	# Allow if not equal.
	NE = 0x02

	# Allow if less than.
	LT = 0x03

	# Allow if less than or equal.
	LE = 0x04

	# Allow if greater than.
	GT = 0x05

	# Allow if greater than or equal.
	GE = 0x06
