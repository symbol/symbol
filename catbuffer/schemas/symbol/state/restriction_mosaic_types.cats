import "types.cats"
import "restriction_mosaic/restriction_mosaic_types.cats"

# type of mosaic restriction entry
enum MosaicRestrictionEntryType : uint8
	# address restriction
	ADDRESS = 0

	# global (mosaic) restriction
	GLOBAL = 1

# layout for mosaic address restriction key-value pair
struct AddressKeyValue
	# key for value
	key = MosaicRestrictionKey

	# value associated by key
	value = uint64

# binary layout for mosaic address restriction key-value set
struct AddressKeyValueSet
	# number of key value pairs
	key_value_count = uint8

	# key value array
	@sort_key(key)
	keys = array(AddressKeyValue, key_value_count)

# binary layout of restriction rule being applied
struct RestrictionRule
	# identifier of the mosaic providing the restriction key
	reference_mosaic_id = MosaicId

	# restriction value
	restriction_value = uint64

	# restriction type
	restriction_type = MosaicRestrictionType

# binary layout for a global key-value
struct GlobalKeyValue
	# key associated with a restriction rule
	key = MosaicRestrictionKey

	# restriction rule (the value) associated with a key
	restriction_rule = RestrictionRule

# binary layout for a global restriction key-value set
struct GlobalKeyValueSet
	# number of key value pairs
	key_value_count = uint8

	# key value array
	@sort_key(key)
	keys = array(GlobalKeyValue, key_value_count)
