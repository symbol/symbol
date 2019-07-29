using MosaicNonce = uint32

# mosaic property flags
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

# available mosaic property ids
enum MosaicPropertyId : uint8
	# mosaic duration
	duration = 0x02

# mosaic property compose of an id and a value
struct MosaicProperty
	# mosaic property id
	id = MosaicPropertyId

	# mosaic property value
	value = uint64

# supply change directions
enum MosaicSupplyChangeDirection : uint8
	# decreases the supply
	decrease = 0x00

	# increases the supply
	increase = 0x01
