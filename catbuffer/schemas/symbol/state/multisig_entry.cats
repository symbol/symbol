import "types.cats"
import "state/state_header.cats"

# binary layout for a multisig entry
struct MultisigEntry
	inline StateHeader

	# minimum approval for modifications
	min_approval = uint32

	# minimum approval for removal
	min_removal = uint32

	# account address
	account_address = Address

	# number of cosignatories
	cosignatory_addresses_count = uint64

	# cosignatories for account
	cosignatory_addresses = array(Address, cosignatory_addresses_count)

	# number of other accounts for which the entry is cosignatory
	multisig_addresses_count = uint64

	# accounts for which the entry is cosignatory
	multisig_addresses = array(Address, multisig_addresses_count)
