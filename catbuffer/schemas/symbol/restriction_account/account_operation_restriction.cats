import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account operation restriction transaction
struct AccountOperationRestrictionTransactionBody
	# account restriction flags
	restrictionFlags = AccountRestrictionFlags

	# number of account restriction additions
	restrictionAdditionsCount = uint8

	# number of account restriction deletions
	restrictionDeletionsCount = uint8

	# reserved padding to align restrictionAdditions on 8-byte boundary
	accountRestrictionTransactionBody_Reserved1 = uint32

	# account restriction additions
	restrictionAdditions = array(EntityType, restrictionAdditionsCount)

	# account restriction deletions
	restrictionDeletions = array(EntityType, restrictionDeletionsCount)

# binary layout for a non-embedded account operation restriction transaction
struct AccountOperationRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4350

	inline Transaction
	inline AccountOperationRestrictionTransactionBody

# binary layout for an embedded account operation restriction transaction
struct EmbeddedAccountOperationRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4350

	inline EmbeddedTransaction
	inline AccountOperationRestrictionTransactionBody
