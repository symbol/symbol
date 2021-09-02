import "types.cats"

using ScopedMetadataKey = uint64

# enum for the different types of metadata
enum MetadataType : uint8
	# account metadata
	ACCOUNT = 0

	# mosaic metadata
	MOSAIC = 1

	# namespace metadata
	NAMESPACE = 2

# binary layout of a metadata entry value
struct MetadataValue
	# size of the value
	size = uint16

	# data of the value
	data = array(uint8, size)
