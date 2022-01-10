import "state/restriction_mosaic_types.cats"
import "state/state_header.cats"

# binary layout for a mosaic restriction
struct MosaicAddressRestrictionEntry
	# identifier of the mosaic to which the restriction applies
	mosaic_id = MosaicId

	# address being restricted
	address = Address

	# address key value restriction set
	key_pairs = AddressKeyValueSet

# binary layout for a mosaic restriction
struct MosaicGlobalRestrictionEntry
	# identifier of the mosaic to which the restriction applies
	mosaic_id = MosaicId

	# global key value restriction set
	key_pairs = GlobalKeyValueSet

# binary layout for a mosaic restriction
struct MosaicRestrictionEntry
	inline StateHeader

	# type of restriction being placed upon the entity
	entry_type = MosaicRestrictionEntryType

	# address restriction rule
	address_entry = MosaicAddressRestrictionEntry if ADDRESS equals entry_type

	# global mosaic rule
	global_entry = MosaicGlobalRestrictionEntry if GLOBAL equals entry_type
