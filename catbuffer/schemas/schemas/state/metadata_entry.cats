import "state/metadata_entry_types.cats"
import "namespace/namespace_types.cats"

# binary layout of a metadata entry
struct MetadataEntry
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
