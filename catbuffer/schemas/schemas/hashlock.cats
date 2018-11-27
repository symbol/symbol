import "transaction.cats"

# binary layout for a hash lock transaction
struct HashLockTransactionBody
	# transaction mosaic
	mosaic = Mosaic
	# number of blocks for which a lock should be valid
	duration = BlockDuration
	# lock hash
	hash = Hash256

# binary layout for a non-embedded hash lock transaction
struct HashLockTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4148

	inline Transaction
	inline HashLockTransactionBody

# binary layout for an embedded hash lock transaction
struct EmbeddedHashLockTransaction
	inline EmbeddedTransaction
	inline HashLockTransactionBody
