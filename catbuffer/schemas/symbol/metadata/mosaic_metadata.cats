import "transaction.cats"

# binary layout for a mosaic metadata transaction
struct MosaicMetadataTransactionBody
	# metadata target address
	targetAddress = UnresolvedAddress

	# metadata key scoped to source, target and type
	scopedMetadataKey = uint64

	# target mosaic identifier
	targetMosaicId = UnresolvedMosaicId

	# change in value size in bytes
	valueSizeDelta = int16

	# value size in bytes
	valueSize = uint16

	# difference between existing value and new value
	# \note when there is no existing value, new value is same this value
	# \note when there is an existing value, new value is calculated as xor(previous-value, value)
	value = array(byte, valueSize)

# binary layout for a non-embedded mosaic metadata transaction
struct MosaicMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4244

	inline Transaction
	inline MosaicMetadataTransactionBody

# binary layout for an embedded mosaic metadata transaction
struct EmbeddedMosaicMetadataTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4244

	inline EmbeddedTransaction
	inline MosaicMetadataTransactionBody
