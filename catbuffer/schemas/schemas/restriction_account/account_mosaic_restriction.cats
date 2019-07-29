import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account mosaic restriction transaction
struct AccountMosaicRestrictionTransactionBody
	# account restriction type
	restrictionType = AccountRestrictionType

	# number of modifications
	modificationsCount = uint8

	# account restriction modifications
	modifications = array(AccountMosaicRestrictionModification, modificationsCount)

# binary layout for a non-embedded account mosaic restriction transaction
struct AccountMosaicRestrictionTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4250

	inline Transaction
	inline AccountMosaicRestrictionTransactionBody

# binary layout for an embedded account mosaic restriction transaction
struct EmbeddedAccountMosaicRestrictionTransaction
	inline EmbeddedTransaction
	inline AccountMosaicRestrictionTransactionBody
