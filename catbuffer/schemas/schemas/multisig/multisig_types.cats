import "types.cats"

# cosignatory modification action
enum CosignatoryModificationAction : uint8
	# add cosignatory
	add = 0x00

	# remove cosignatory
	del = 0x01

# binary layout for a cosignatory modification
struct CosignatoryModification
	# modification action
	modificationAction = CosignatoryModificationAction

	# cosignatory account public key
	cosignatoryPublicKey = Key
