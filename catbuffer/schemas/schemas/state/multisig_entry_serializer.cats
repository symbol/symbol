import "types.cats"

# binary layout for a multisig entry
struct MultisigEntry

	# new key entry for account
	entryKey = Key

	# minimum approval for modifications
	minApproval = int8

	# minimum approval for removal
	minRemoval = int8

	# number of cosignatories
	cosignatoryCount = uint8

	# number of other accounts for which the entry is cosigner
	multisigCount = uint8

	# cosignatories for account
	cosignatories = array(Key, cosignatoryCount)

	# accounts for which the entry is cosigner
	multisigAccounts = array(Key, multisigCount)
