import "types.cats"

# lock status for lock transaction
enum LockStatus : uint8
	# lock is unused
	UNUSED = 0

	# lock was already used
	USED = 1
