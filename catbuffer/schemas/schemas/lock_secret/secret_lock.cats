import "lock_secret/lock_secret_types.cats"
import "transaction.cats"

# binary layout for a secret lock transaction
struct SecretLockTransactionBody
	# locked mosaic
	mosaic = UnresolvedMosaic

	# number of blocks for which a lock should be valid
	duration = BlockDuration

	# hash algorithm
	hashAlgorithm = LockHashAlgorithm

	# secret
	secret = Hash256

	# locked mosaic recipient address
	recipientAddress = UnresolvedAddress

# binary layout for a non-embedded secret lock transaction
struct SecretLockTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4152

	inline Transaction
	inline SecretLockTransactionBody

# binary layout for an embedded secret lock transaction
struct EmbeddedSecretLockTransaction
	inline EmbeddedTransaction
	inline SecretLockTransactionBody
