import "restriction_account/account_restriction_types.cats"
import "transaction.cats"

# account operation restriction modification
struct AccountOperationRestrictionModification
	inline AccountRestrictionModification
	value = EntityType

# binary layout for an account operation restriction transaction
struct AccountOperationRestrictionTransactionBody
	# account restriction type
	restrictionType = AccountRestrictionType

	# number of modifications
	modificationsCount = uint8

	# account restriction modifications
	modifications = array(AccountOperationRestrictionModification, modificationsCount)

# binary layout for a non-embedded account operation restriction transaction
struct AccountOperationRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4153

	inline Transaction
	inline AccountOperationRestrictionTransactionBody

# binary layout for an embedded account operation restriction transaction
struct EmbeddedAccountOperationRestrictionTransaction
	inline EmbeddedTransaction
	inline AccountOperationRestrictionTransactionBody
