import "types.cats"

# lock status for lock transaction
enum LockStatus : uint8
	# lock is unused
	unused = 0

	# lock was already used
	used = 1
