using MosaicRestrictionKey = uint64

# enumeration of mosaic restriction types
enum MosaicRestrictionType : uint8
	# uninitialized value indicating no restriction
	NONE = 0x00

	# allow if equal
	EQ = 0x01

	# allow if not equal
	NE = 0x02

	# allow if less than
	LT = 0x03

	# allow if less than or equal
	LE = 0x04

	# allow if greater than
	GT = 0x05

	# allow if greater than or equal
	GE = 0x06
