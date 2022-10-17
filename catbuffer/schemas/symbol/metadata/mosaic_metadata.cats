import "transaction.cats"

# Shared content between MosaicMetadataTransaction and EmbeddedMosaicMetadataTransaction.
inline struct MosaicMetadataTransactionBody
	# Account owning the mosaic whose metadata should be modified.
	target_address = UnresolvedAddress

	# Metadata key scoped to source, target and type.
	scoped_metadata_key = uint64

	# Mosaic whose metadata should be modified.
	target_mosaic_id = UnresolvedMosaicId

	# Change in value size in bytes, compared to previous size.
	value_size_delta = int16

	# Size in bytes of the `value` array.
	value_size = uint16

	# Difference between existing value and new value.
	# \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`.
	# \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array.
	value = array(uint8, value_size)

# Associate a key-value state ([metadata](/concepts/metadata.html)) to a **mosaic** (V1, latest).
#
# Compare to AccountMetadataTransaction and NamespaceMetadataTransaction.
struct MosaicMetadataTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_METADATA)

	inline Transaction
	inline MosaicMetadataTransactionBody

# Embedded version of MosaicMetadataTransaction (V1, latest).
struct EmbeddedMosaicMetadataTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_METADATA)

	inline EmbeddedTransaction
	inline MosaicMetadataTransactionBody
