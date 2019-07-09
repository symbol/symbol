import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a namespace metadata transaction
struct NamespaceMetadataTransactionBody
	# public key of the metadata target
	targetPublicKey = Key

	# metadata key scoped to source, target and type
	scopedMetadataKey = uint64

	# namespace id
	targetId = NamespaceId

	# change in value size in bytes
	valueSizeDelta = int16

	# value size in bytes
	valueSize = uint16

	# value data
	value = array(byte, valueSize)

# binary layout for a non-embedded namespace metadata transaction
struct NamespaceMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4344

	inline Transaction
	inline NamespaceMetadataTransactionBody

# binary layout for an embedded namespace metadata transaction
struct EmbeddedNamespaceMetadataTransaction
	inline EmbeddedTransaction
	inline NamespaceMetadataTransactionBody
