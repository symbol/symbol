import "types.cats"

# enumeration of cosignatory modification actions
enum CosignatoryModificationAction : uint8
	# remove cosignatory
	del = 0x00

	# add cosignatory
	add = 0x01

# binary layout for a cosignatory modification
struct CosignatoryModification
	# modification action
	modificationAction = CosignatoryModificationAction

	# cosignatory account public key
	cosignatoryPublicKey = Key
