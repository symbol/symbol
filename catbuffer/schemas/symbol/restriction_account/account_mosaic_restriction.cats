import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account mosaic restriction transaction
struct AccountMosaicRestrictionTransactionBody
	# account restriction flags
	restriction_flags = AccountRestrictionFlags

	# number of account restriction additions
	restriction_additions_count = uint8

	# number of account restriction deletions
	restriction_deletions_count = uint8

	# reserved padding to align restrictionAdditions on 8-byte boundary
	account_restriction_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# account restriction additions
	restriction_additions = array(UnresolvedMosaicId, restriction_additions_count)

	# account restriction deletions
	restriction_deletions = array(UnresolvedMosaicId, restriction_deletions_count)

# binary layout for a non-embedded account mosaic restriction transaction
struct AccountMosaicRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_MOSAIC_RESTRICTION)

	inline Transaction
	inline AccountMosaicRestrictionTransactionBody

# binary layout for an embedded account mosaic restriction transaction
struct EmbeddedAccountMosaicRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_MOSAIC_RESTRICTION)

	inline EmbeddedTransaction
	inline AccountMosaicRestrictionTransactionBody
