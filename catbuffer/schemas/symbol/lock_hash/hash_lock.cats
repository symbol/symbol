import "transaction.cats"

# binary layout for a hash lock transaction
struct HashLockTransactionBody
	# lock mosaic
	mosaic = UnresolvedMosaic

	# number of blocks for which a lock should be valid
	duration = BlockDuration

	# lock hash
	hash = Hash256

# binary layout for a non-embedded hash lock transaction
struct HashLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, HASH_LOCK)

	inline Transaction
	inline HashLockTransactionBody

# binary layout for an embedded hash lock transaction
struct EmbeddedHashLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, HASH_LOCK)

	inline EmbeddedTransaction
	inline HashLockTransactionBody
