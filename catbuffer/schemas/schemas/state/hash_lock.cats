import "state/lock_info.cats"

# binary layout for hash lock transaction info
struct HashLockInfo
	# sender public key
	senderPublicKey = Key

	# mosaic associated with lock
	mosaic = Mosaic

	# height at which the lock expires
	endHeight = Height

	# flag indicating whether or not the lock was already used
	status = LockStatus

	# hash
	hash = Hash256
