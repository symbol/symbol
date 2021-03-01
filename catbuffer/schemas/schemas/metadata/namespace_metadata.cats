import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a namespace metadata transaction
struct NamespaceMetadataTransactionBody
	# metadata target address
	targetAddress = UnresolvedAddress

	# metadata key scoped to source, target and type
	scopedMetadataKey = uint64

	# target namespace identifier
	targetNamespaceId = NamespaceId

	# change in value size in bytes
	valueSizeDelta = int16

	# value size in bytes
	valueSize = uint16

	# difference between existing value and new value
	# \note when there is no existing value, new value is same this value
	# \note when there is an existing value, new value is calculated as xor(previous-value, value)
	value = array(byte, valueSize)

# binary layout for a non-embedded namespace metadata transaction
struct NamespaceMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4344

	inline Transaction
	inline NamespaceMetadataTransactionBody

# binary layout for an embedded namespace metadata transaction
struct EmbeddedNamespaceMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4344

	inline EmbeddedTransaction
	inline NamespaceMetadataTransactionBody
