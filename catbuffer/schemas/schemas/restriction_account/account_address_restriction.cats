import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account address restriction transaction
struct AccountAddressRestrictionTransactionBody
	# account restriction flags
	restrictionFlags = AccountRestrictionFlags

	# number of account restriction additions
	restrictionAdditionsCount = uint8

	# number of account restriction deletions
	restrictionDeletionsCount = uint8

	# reserved padding to align restrictionAdditions on 8-byte boundary
	accountRestrictionTransactionBody_Reserved1 = uint32

	# account restriction additions
	restrictionAdditions = array(UnresolvedAddress, restrictionAdditionsCount)

	# account restriction deletions
	restrictionDeletions = array(UnresolvedAddress, restrictionDeletionsCount)

# binary layout for a non-embedded account address restriction transaction
struct AccountAddressRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4150

	inline Transaction
	inline AccountAddressRestrictionTransactionBody

# binary layout for an embedded account address restriction transaction
struct EmbeddedAccountAddressRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4150

	inline EmbeddedTransaction
	inline AccountAddressRestrictionTransactionBody
