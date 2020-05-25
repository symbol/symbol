import "types.cats"
import "mosaic/mosaic_types.cats"

# binary layout for mosaic properties
struct MosaicProperties
	# mosaic flags
	flags = MosaicFlags

	# mosaic divisibility
	divisibility = uint8

	# mosaic duration
	duration = BlockDuration

# binary layout for mosaic definition
struct MosaicDefinition
	# block height
	startHeight = Height

	# mosaic owner
	ownerAddress = Address

	# revision
	revision = uint32

	# properties
	properties = MosaicProperties
