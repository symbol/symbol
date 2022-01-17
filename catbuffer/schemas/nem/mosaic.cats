import "types.cats"

# binary layout for a namespace id
struct NamespaceId
	# [__value__] name
	#
	# [size] name size
	name = inline SizePrefixedString

# binary layout for a mosaic id
@is_size_implicit
struct MosaicId
	# namespace id
	namespace_id = NamespaceId

	# [__value__] name
	#
	# [size] name size
	name = inline SizePrefixedString

# binary layout for a mosaic
@is_size_implicit
struct Mosaic
	# mosaic id size
	mosaic_id_size = sizeof(uint32, mosaic_id)

	# mosaic id
	mosaic_id = MosaicId

	# quantity
	amount = Amount

# binary layout for a mosaic with a size prefixed size
struct SizePrefixedMosaic
	# mosaic size
	mosaic_size = sizeof(uint32, mosaic)

	# mosaic
	mosaic = Mosaic
