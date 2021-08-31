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
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, hash_lock)

	inline Transaction
	inline HashLockTransactionBody

# binary layout for an embedded hash lock transaction
struct EmbeddedHashLockTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, hash_lock)

	inline EmbeddedTransaction
	inline HashLockTransactionBody
