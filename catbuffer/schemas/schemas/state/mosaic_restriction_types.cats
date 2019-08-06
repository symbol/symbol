import "types.cats"
import "restriction_mosaic/restriction_mosaic_types.cats"

# mosaic restriction type
enum EntryType : uint8

	# address restriction
	address = 0

	# global mosaic restriction
	global = 1


# binary layout of restriction rule being applied
struct RestrictionRule 

	# identifier of the mosaic providing the restriction key
	referenceMosaicId = MosaicId

	# restriction value
	restrictionValue = uint64

	# restriction type
	restrictionType = MosaicRestrictionType


# layout for a key-value pair
struct KeyValue 

    # key for value
	key = uint64

    # value associated by key
	value = uint64

# binary layout for a global key-value
struct GlobalKeyValue

    # key associated with a restriction rule
	key = uint64

    # restriction rule (the value) associated with a key
	restrictionRule = RestrictionRule


# binary layout for an address key-value set
struct KeyValueSet

    # key array size
	keyValueSize = uint8

    # key value array
	keys = array(KeyValue, keyValueSize)

# binary layout for a global restriction key-value set
struct GlobalKeyValueSet 

    # key array size
	keyValueSize = uint8

    # key value array
	keys = array(GlobalKeyValue, keyValueSize)
