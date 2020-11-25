import "state/restriction_mosaic_types.cats"
import "state/state_header.cats"

# binary layout for a mosaic restriction
struct MosaicAddressRestrictionEntry
	# identifier of the mosaic to which the restriction applies
	mosaicId = MosaicId

	# address being restricted
	address = Address

	# address key value restriction set
	keyPairs = AddressKeyValueSet

# binary layout for a mosaic restriction
struct MosaicGlobalRestrictionEntry
	# identifier of the mosaic to which the restriction applies
	mosaicId = MosaicId

	# global key value restriction set
	keyPairs = GlobalKeyValueSet

# binary layout for a mosaic restriction
struct MosaicRestrictionEntry
	inline StateHeader

	# type of restriction being placed upon the entity
	entryType = MosaicRestrictionEntryType

	# address restriction rule
	addressEntry = MosaicAddressRestrictionEntry if entryType equals address

	# global mosaic rule
	globalEntry = MosaicGlobalRestrictionEntry if entryType equals global
