import "lock_secret/lock_secret_types.cats"
import "state/lock_info.cats"
import "state/state_header.cats"

# binary layout for serialized lock transaction
struct SecretLockInfo
	inline StateHeader

	# owner address
	owner_address = Address

	# mosaic associated with lock
	mosaic = Mosaic

	# height at which the lock expires
	end_height = Height

	# flag indicating whether or not the lock was already used
	status = LockStatus

	# hash algorithm
	hash_algorithm = LockHashAlgorithm

	# transaction secret
	secret = Hash256

	# transaction recipient
	recipient = Address
