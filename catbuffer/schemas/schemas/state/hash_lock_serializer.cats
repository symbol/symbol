import "state/lock_info_serializer.cats"

# binary layout for hash lock transaction info
struct HashLockInfo
	# account with lock hash
	account = Key

	# mosaic id
	mosaicId = MosaicId

	# mosaic amount held within the transaction
	amount = Amount

	# block height
	height = Height

	# status that indicates whether the lock was used or not
	lockStatus = LockStatus

	# transaction hash
	hash = Hash256
