import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account mosaic restriction transaction
struct AccountMosaicRestrictionTransactionBody
	# account restriction flags
	restrictionFlags = AccountRestrictionFlags

	# number of account restriction additions
	restrictionAdditionsCount = uint8

	# number of account restriction deletions
	restrictionDeletionsCount = uint8

	# reserved padding to align restrictionAdditions on 8-byte boundary
	accountRestrictionTransactionBody_Reserved1 = make_reserved(uint32, 0)

	# account restriction additions
	restrictionAdditions = array(UnresolvedMosaicId, restrictionAdditionsCount)

	# account restriction deletions
	restrictionDeletions = array(UnresolvedMosaicId, restrictionDeletionsCount)

# binary layout for a non-embedded account mosaic restriction transaction
struct AccountMosaicRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_mosaic_restriction)

	inline Transaction
	inline AccountMosaicRestrictionTransactionBody

# binary layout for an embedded account mosaic restriction transaction
struct EmbeddedAccountMosaicRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_mosaic_restriction)

	inline EmbeddedTransaction
	inline AccountMosaicRestrictionTransactionBody
