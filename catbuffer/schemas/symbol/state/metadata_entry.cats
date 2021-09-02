import "state/metadata_entry_types.cats"
import "state/state_header.cats"
import "namespace/namespace_types.cats"

# binary layout of a metadata entry
struct MetadataEntry
	inline StateHeader

	# metadata source address (provider)
	source_address = Address

	# metadata target address
	target_address = Address

	# metadata key scoped to source, target and type
	scoped_metadata_key = ScopedMetadataKey

	# target id
	target_id = uint64

	# metadata type
	metadata_type = MetadataType

	# value
	value = MetadataValue
