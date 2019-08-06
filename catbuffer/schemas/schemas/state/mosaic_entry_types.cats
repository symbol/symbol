import "types.cats"

# available mosaic property ids enum
enum MosaicPropertyId : uint8 
	# mosaic flags
	flags = 0

	# mosaic divisibility
	divisibility = 1

	# mosaic duration
	duration = 2

	sentinelPropertyId = 3

struct MosaicProperty

	# mosaic property id
	id = MosaicPropertyId

	# value 
	value = uint64



# binary layout for mosaic definition
struct MosaicDefinition
	# block height
	height = Height

	# mosaic owner
	owner = Key

	# revision
	revision = uint32

	# number of elements in optional properties
	propertiesCount = uint8

	# optional properties
	properties = array(MosaicProperty, propertiesCount, sort_key=id)
