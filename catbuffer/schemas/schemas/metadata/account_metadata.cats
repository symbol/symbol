import "transaction.cats"

# binary layout for an account metadata transaction
struct AccountMetadataTransactionBody
	# public key of the metadata target
	targetPublicKey = Key

	# metadata key scoped to source, target and type
	scopedMetadataKey = uint64

	# change in value size in bytes
	valueSizeDelta = int16

	# value size in bytes
	valueSize = uint16

	# value data
	value = array(byte, valueSize)

# binary layout for a non-embedded account metadata transaction
struct AccountMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4144

	inline Transaction
	inline AccountMetadataTransactionBody

# binary layout for an embedded account metadata transaction
struct EmbeddedAccountMetadataTransaction
	inline EmbeddedTransaction
	inline AccountMetadataTransactionBody
