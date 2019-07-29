import "types.cats"

# cosignatory modification type
enum CosignatoryModificationType : uint8
	# add cosignatory
	add = 0x00

	# remove cosignatory
	del = 0x01

# binary layout for a cosignatory modification
struct CosignatoryModification
	# modification type
	modificationType = CosignatoryModificationType

	# cosignatory account public key
	cosignatoryPublicKey = Key
