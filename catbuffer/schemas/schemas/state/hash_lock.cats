import "state/lock_info.cats"

# binary layout for hash lock transaction info
struct HashLockInfo
	# owner address
	ownerAddress = Address

	# mosaic associated with lock
	mosaic = Mosaic

	# height at which the lock expires
	endHeight = Height

	# flag indicating whether or not the lock was already used
	status = LockStatus

	# hash
	hash = Hash256
