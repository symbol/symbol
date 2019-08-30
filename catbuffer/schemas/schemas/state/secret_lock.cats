import "lock_secret/lock_secret_types.cats"
import "state/lock_info.cats"

# binary layout for serialized lock transaction
struct SecretLockInfo
	# sender public key
	senderPublicKey = Key

	# mosaic associated with lock
	mosaic = Mosaic

	# height at which the lock expires
	endHeight = Height

	# flag indicating whether or not the lock was already used
	status = LockStatus

	# hash algorithm
	hashAlgorithm = LockHashAlgorithm

	# transaction secret
	secret = Hash256

	# transaction recipient
	recipient = Address
