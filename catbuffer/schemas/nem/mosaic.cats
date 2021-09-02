import "types.cats"

# binary layout for a namespace id
struct NamespaceId
	# size
	size = uint32

	# name
	name = array(int8, size)

# binary layout for a mosaic id
struct MosaicId
	# namespace id
	namespace_id = NamespaceId

	# size
	size = uint32

	# name
	name = array(int8, size)

# binary layout for a mosaic
struct Mosaic
	# mosaic id
	mosaic_id = MosaicId

	# quantity
	amount = Amount
