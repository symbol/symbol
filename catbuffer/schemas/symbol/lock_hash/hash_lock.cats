import "transaction.cats"

# Shared content between HashLockTransaction and EmbeddedHashLockTransaction.
inline struct HashLockTransactionBody
	# Locked mosaic.
	mosaic = UnresolvedMosaic

	# Number of blocks for which a lock should be valid.
	#
	# The default maximum is 48h (See the `maxHashLockDuration` network property).
	duration = BlockDuration

	# Hash of the AggregateBondedTransaction to be confirmed before unlocking the mosaics.
	hash = Hash256

# Lock a deposit needed to announce an AggregateBondedTransaction (V1, latest).
#
# An AggregateBondedTransaction consumes network resources as it is stored in every node's partial cache while
# it waits to be fully signed. To avoid spam attacks a HashLockTransaction must be announced and confirmed
# before an AggregateBondedTransaction can be announced. The HashLockTransaction locks a certain amount of funds
# (**10** XYM by default) until the aggregate is signed.
#
# Upon completion of the aggregate, the locked funds become available again to the account that signed the HashLockTransaction.
#
# If the lock expires before the aggregate is signed by all cosignatories (**48h by default),
# the locked funds become a reward collected by the block harvester at the height where the lock expires.
#
# \note It is not necessary to sign the aggregate and its HashLockTransaction with the same account.
# For example, if Bob wants to announce an aggregate and does not have enough funds to announce a HashLockTransaction,
# he can ask Alice to announce the lock transaction for him by sharing the signed AggregateTransaction hash.
struct HashLockTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, HASH_LOCK)

	inline Transaction
	inline HashLockTransactionBody

# Embedded version of HashLockTransaction.
struct EmbeddedHashLockTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, HASH_LOCK)

	inline EmbeddedTransaction
	inline HashLockTransactionBody
