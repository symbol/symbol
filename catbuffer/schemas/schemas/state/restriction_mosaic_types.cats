import "types.cats"
import "restriction_mosaic/restriction_mosaic_types.cats"

# type of mosaic restriction entry
enum MosaicRestrictionEntryType : uint8
	# address restriction
	address = 0

	# global (mosaic) restriction
	global = 1

# layout for mosaic address restriction key-value pair
struct AddressKeyValue
	# key for value
	key = MosaicRestrictionKey

	# value associated by key
	value = uint64

# binary layout for mosaic address restriction key-value set
struct AddressKeyValueSet
	# number of key value pairs
	keyValueCount = uint8

	# key value array
	keys = array(AddressKeyValue, keyValueCount, sort_key=key)

# binary layout of restriction rule being applied
struct RestrictionRule
	# identifier of the mosaic providing the restriction key
	referenceMosaicId = MosaicId

	# restriction value
	restrictionValue = uint64

	# restriction type
	restrictionType = MosaicRestrictionType

# binary layout for a global key-value
struct GlobalKeyValue
	# key associated with a restriction rule
	key = MosaicRestrictionKey

	# restriction rule (the value) associated with a key
	restrictionRule = RestrictionRule

# binary layout for a global restriction key-value set
struct GlobalKeyValueSet
	# number of key value pairs
	keyValueCount = uint8

	# key value array
	keys = array(GlobalKeyValue, keyValueCount, sort_key=key)
