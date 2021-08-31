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
	value = array(uint8, valueSize)

# binary layout for a non-embedded mosaic metadata transaction
struct MosaicMetadataTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_metadata)

	inline Transaction
	inline MosaicMetadataTransactionBody

# binary layout for an embedded mosaic metadata transaction
struct EmbeddedMosaicMetadataTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_metadata)

	inline EmbeddedTransaction
	inline MosaicMetadataTransactionBody
