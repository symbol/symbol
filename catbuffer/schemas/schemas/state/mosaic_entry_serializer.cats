import "state/mosaic_entry_types.cats"

# binary layout for mosaic entry serializer
struct MosaicEntry
	# entry id
	mosaicId = MosaicId

	# total supply amount
	supply = Amount

	# definition comprised of entry properties
	definition = MosaicDefinition
