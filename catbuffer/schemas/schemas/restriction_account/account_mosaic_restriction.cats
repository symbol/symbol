import "restriction_account/account_restriction_types.cats"
import "transaction.cats"

# account mosaic restriction modification
struct AccountMosaicRestrictionModification
	inline AccountRestrictionModification
	value = MosaicId

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
