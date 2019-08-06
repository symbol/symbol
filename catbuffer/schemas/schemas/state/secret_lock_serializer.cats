import "lock_secret/lock_secret_types.cats"
import "types.cats"

# binary layout for serialized lock transaction
struct SecretLockInfo
	# hash algorithm
	hashAlgorithm = LockHashAlgorithm

	# transaction secret
	secret = Hash256

	# transaction recipient
	recipient = Address
