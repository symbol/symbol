using MosaicNonce = uint32

# enumeration of mosaic property flags
enum MosaicFlags : uint8
	# no flags present
	NONE = 0x00

	# mosaic supports supply changes even when mosaic owner owns partial supply
	SUPPLY_MUTABLE = 0x01

	# mosaic supports transfers between arbitrary accounts
	# \note when not set, mosaic can only be transferred to and from mosaic owner
	TRANSFERABLE = 0x02

	# mosaic supports custom restrictions configured by mosaic owner
	RESTRICTABLE = 0x04

# enumeration of mosaic supply change actions
enum MosaicSupplyChangeAction : uint8
	# decreases the supply
	DECREASE = 0x00

	# increases the supply
	INCREASE = 0x01
