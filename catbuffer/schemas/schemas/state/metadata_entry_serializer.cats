import "state/metadata_entry_types.cats"

# binary layout of a metadata entry serializer
struct MetadataEntry
	# key in which the data is being signed
	sourcePublicKey = Key

	# key in which data is being applied to
	targetPublicKey = Key

	# key scoped to source, target and type
	scopedKey = uint64

	# uint64 id of the entity receiving the data
	targetId = uint64

	# entity that has receiving the entry the entry
	type = MetadataType

	# value
	value = MetadataValue
