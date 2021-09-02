import "state/mosaic_entry_types.cats"
import "state/state_header.cats"

# binary layout for mosaic entry
struct MosaicEntry
	inline StateHeader

	# entry id
	mosaic_id = MosaicId

	# total supply amount
	supply = Amount

	# definition comprised of entry properties
	definition = MosaicDefinition
