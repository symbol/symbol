import "types.cats"

# binary layout for a multisig entry
struct MultisigEntry
	# minimum approval for modifications
	minApproval = uint8

	# minimum approval for removal
	minRemoval = uint8

	# account public key
	accountPublicKey = Key

	# number of cosignatories
	cosignatoryPublicKeysCount = uint64

	# cosignatories for account
	cosignatoryPublicKeys = array(Key, cosignatoryPublicKeysCount)

	# number of other accounts for which the entry is cosignatory
	multisigPublicKeysCount = uint64

	# accounts for which the entry is cosignatory
	multisigPublicKeys = array(Key, multisigPublicKeysCount)
