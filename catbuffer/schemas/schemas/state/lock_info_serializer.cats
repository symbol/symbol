import "types.cats"

# lock status for lock transaction
enum LockStatus : uint8
	# lock is unused
	unused = 0

	# lock was already used
	used = 1

# binary layout for serialized lock info
struct LockInfo
	# id of the mosaic
	mosaicId = MosaicId

	# mosaic amount held within the transaction
	amount = Amount

	# block height
	height = Height

	# transaction status
	status = LockStatus
