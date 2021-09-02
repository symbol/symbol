import "transaction.cats"

# binary layout for a mosaic metadata transaction
struct MosaicMetadataTransactionBody
	# metadata target address
	target_address = UnresolvedAddress

	# metadata key scoped to source, target and type
	scoped_metadata_key = uint64

	# target mosaic identifier
	target_mosaic_id = UnresolvedMosaicId

	# change in value size in bytes
	value_size_delta = int16

	# value size in bytes
	value_size = uint16

	# difference between existing value and new value
	# \note when there is no existing value, new value is same this value
	# \note when there is an existing value, new value is calculated as xor(previous-value, value)
	value = array(uint8, value_size)

# binary layout for a non-embedded mosaic metadata transaction
struct MosaicMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_METADATA)

	inline Transaction
	inline MosaicMetadataTransactionBody

# binary layout for an embedded mosaic metadata transaction
struct EmbeddedMosaicMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_METADATA)

	inline EmbeddedTransaction
	inline MosaicMetadataTransactionBody
