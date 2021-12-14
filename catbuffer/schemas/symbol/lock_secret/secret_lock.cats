import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# Shared content between SecretLockTransaction and EmbeddedSecretLockTransaction.
inline struct SecretLockTransactionBody
	# Address that receives the funds once successfully unlocked by a SecretProofTransaction.
	recipient_address = UnresolvedAddress

	# Hashed proof.
	secret = Hash256

	# Locked mosaics.
	mosaic = UnresolvedMosaic

	# Number of blocks to wait for the SecretProofTransaction.
	duration = BlockDuration

	# Algorithm used to hash the proof.
	hash_algorithm = LockHashAlgorithm

# Start a token swap between different chains.
#
# Use a SecretLockTransaction to transfer mosaics between two accounts.
# The mosaics sent remain locked until a valid SecretProofTransaction unlocks them.
#
# The default expiration date is **365 days** after announcement (See the `maxSecretLockDuration` network property).
# If the lock expires before a valid SecretProofTransaction is announced the locked
# amount goes back to the initiator of the SecretLockTransaction.
struct SecretLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_LOCK)

	inline Transaction
	inline SecretLockTransactionBody

# Embedded version of SecretLockTransaction.
struct EmbeddedSecretLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_LOCK)

	inline EmbeddedTransaction
	inline SecretLockTransactionBody
