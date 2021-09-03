import "types.cats"

# binary layout for a namespace id
struct NamespaceId
	# [__value__] name
	#
	# [size] name size
	name = inline SizePrefixedString

# binary layout for a mosaic id
struct MosaicId
	# namespace id
	namespace_id = NamespaceId

	# [__value__] name
	#
	# [size] name size
	name = inline SizePrefixedString

# binary layout for a mosaic
struct Mosaic
	# mosaic id
	mosaic_id = MosaicId

	# quantity
	amount = Amount
