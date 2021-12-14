import "transaction.cats"

# Shared content between AccountMetadataTransaction and EmbeddedAccountMetadataTransaction.
inline struct AccountMetadataTransactionBody
	# Account whose metadata should be modified.
	target_address = UnresolvedAddress

	# Metadata key scoped to source, target and type.
	scoped_metadata_key = uint64

	# Change in value size in bytes, compared to previous size.
	value_size_delta = int16

	# Size in bytes of the `value` array.
	value_size = uint16

	# Difference between existing value and new value.
	# \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`.
	# \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array.
	value = array(uint8, value_size)

# Associate a key-value state ([metadata](/concepts/metadata.html)) to an **account**.
#
# \note This transaction must **always** be wrapped in an AggregateTransaction so that a cosignature from
# `target_address` can be provided. Without this cosignature the transaction is invalid.
#
# Compare to MosaicMetadataTransaction and NamespaceMetadataTransaction.
struct AccountMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_METADATA)

	inline Transaction
	inline AccountMetadataTransactionBody

# Embedded version of AccountMetadataTransaction.
struct EmbeddedAccountMetadataTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_METADATA)

	inline EmbeddedTransaction
	inline AccountMetadataTransactionBody
