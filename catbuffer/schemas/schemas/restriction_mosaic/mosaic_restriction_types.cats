# mosaic restriction types
enum MosaicRestrictionType : uint8
	# uninitialized value indicating no restriction
	none = 0

	# allow if equal
	eq = 1

	# allow if not equal
	ne = 2

	# allow if less than
	lt = 3

	# allow if less than or equal
	le = 4

	# allow if greater than
	gt = 5

	# allow if greater than or equal
	ge = 6
