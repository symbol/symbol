using MosaicRestrictionKey = uint64

# enumeration of mosaic restriction types
enum MosaicRestrictionType : uint8
	# uninitialized value indicating no restriction
	none = 0x00

	# allow if equal
	eq = 0x01

	# allow if not equal
	ne = 0x02

	# allow if less than
	lt = 0x03

	# allow if less than or equal
	le = 0x04

	# allow if greater than
	gt = 0x05

	# allow if greater than or equal
	ge = 0x06
