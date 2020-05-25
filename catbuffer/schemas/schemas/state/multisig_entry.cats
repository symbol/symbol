import "types.cats"

# binary layout for a multisig entry
struct MultisigEntry
	# minimum approval for modifications
	minApproval = uint32

	# minimum approval for removal
	minRemoval = uint32

	# account address
	accountAddress = Address

	# number of cosignatories
	cosignatoryAddressesCount = uint64

	# cosignatories for account
	cosignatoryAddresses = array(Address, cosignatoryAddressesCount)

	# number of other accounts for which the entry is cosignatory
	multisigAddressesCount = uint64

	# accounts for which the entry is cosignatory
	multisigAddresses = array(Address, multisigAddressesCount)
