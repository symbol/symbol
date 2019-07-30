using MosaicNonce = uint32

# enumeration of mosaic property flags
enum MosaicFlags : uint8
	# no flags present
	none = 0x00

	# mosaic supports supply changes even when mosaic owner owns partial supply
	supplyMutable = 0x01

	# mosaic supports transfers between arbitrary accounts
	# \note when not set, mosaic can only be transferred to and from mosaic owner
	transferable = 0x02

	# mosaic supports custom restrictions configured by mosaic owner
	restrictable = 0x04

# enumeration of optional mosaic property identifiers
enum MosaicPropertyId : uint8
	# mosaic duration
	duration = 0x02

# mosaic property composed of an identifier and a value
struct MosaicProperty
	# mosaic property identifier
	id = MosaicPropertyId

	# mosaic property value
	value = uint64

# enumeration of mosaic supply change actions
enum MosaicSupplyChangeAction : uint8
	# decreases the supply
	decrease = 0x00

	# increases the supply
	increase = 0x01
