import "transaction.cats"

# binary layout for an account metadata transaction
struct AccountMetadataTransactionBody
	# metadata target address
	targetAddress = UnresolvedAddress

	# metadata key scoped to source, target and type
	scopedMetadataKey = uint64

	# change in value size in bytes
	valueSizeDelta = int16

	# value size in bytes
	valueSize = uint16

	# difference between existing value and new value
	# \note when there is no existing value, new value is same this value
	# \note when there is an existing value, new value is calculated as xor(previous-value, value)
	value = array(uint8, valueSize)

# binary layout for a non-embedded account metadata transaction
struct AccountMetadataTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_metadata)

	inline Transaction
	inline AccountMetadataTransactionBody

# binary layout for an embedded account metadata transaction
struct EmbeddedAccountMetadataTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_metadata)

	inline EmbeddedTransaction
	inline AccountMetadataTransactionBody
