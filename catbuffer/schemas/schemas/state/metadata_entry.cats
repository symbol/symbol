import "state/metadata_entry_types.cats"
import "state/state_header.cats"
import "namespace/namespace_types.cats"

# binary layout of a metadata entry
struct MetadataEntry
	inline StateHeader

	# metadata source address (provider)
	sourceAddress = Address

	# metadata target address
	targetAddress = Address

	# metadata key scoped to source, target and type
	scopedMetadataKey = ScopedMetadataKey

	# target id
	targetId = uint64

	# metadata type
	metadataType = MetadataType

	# value
	value = MetadataValue
