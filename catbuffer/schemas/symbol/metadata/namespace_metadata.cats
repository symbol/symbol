import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a namespace metadata transaction
struct NamespaceMetadataTransactionBody
	# metadata target address
	target_address = UnresolvedAddress

	# metadata key scoped to source, target and type
	scoped_metadata_key = uint64

	# target namespace identifier
	target_namespace_id = NamespaceId

	# change in value size in bytes
	value_size_delta = int16

	# value size in bytes
	value_size = uint16

	# difference between existing value and new value
	# \note when there is no existing value, new value is same this value
	# \note when there is an existing value, new value is calculated as xor(previous-value, value)
	value = array(uint8, value_size)

# binary layout for a non-embedded namespace metadata transaction
struct NamespaceMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_METADATA)

	inline Transaction
	inline NamespaceMetadataTransactionBody

# binary layout for an embedded namespace metadata transaction
struct EmbeddedNamespaceMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, NAMESPACE_METADATA)

	inline EmbeddedTransaction
	inline NamespaceMetadataTransactionBody
