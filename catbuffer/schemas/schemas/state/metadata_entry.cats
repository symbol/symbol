import "state/metadata_entry_types.cats"
import "namespace/namespace_types.cats"

# binary layout of a metadata entry
struct MetadataEntry
	# metadata source public key (provider)
	sourcePublicKey = Key

	# public key of the metadata target
	targetPublicKey = Key

	# metadata key scoped to source, target and type
	scopedMetadataKey = ScopedMetadataKey

	# target id
	targetId = uint64

	# metadata type
	metadataType = MetadataType

	# size of the value
	valueSize = uint16

	# value
	value = MetadataValue
