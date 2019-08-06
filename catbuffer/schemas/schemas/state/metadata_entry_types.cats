import "types.cats"

# enum for the different types of metadata 
enum MetadataType : uint8

	# account metadata
	account = 0

	# mosaic metadata
	mosaic = 1

	# namespace metadata
	namespace = 2


# binary layout of a metadata entry value
struct MetadataValue
	# data size
	size = uint16

	# actual data
	data = uint8
