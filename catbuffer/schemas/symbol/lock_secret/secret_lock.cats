import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# binary layout for a secret lock transaction
struct SecretLockTransactionBody
	# locked mosaic recipient address
	recipient_address = UnresolvedAddress

	# secret
	secret = Hash256

	# locked mosaic
	mosaic = UnresolvedMosaic

	# number of blocks for which a lock should be valid
	duration = BlockDuration

	# hash algorithm
	hash_algorithm = LockHashAlgorithm

# binary layout for a non-embedded secret lock transaction
struct SecretLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_LOCK)

	inline Transaction
	inline SecretLockTransactionBody

# binary layout for an embedded secret lock transaction
struct EmbeddedSecretLockTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, SECRET_LOCK)

	inline EmbeddedTransaction
	inline SecretLockTransactionBody
