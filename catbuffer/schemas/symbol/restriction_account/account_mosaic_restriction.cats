import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# Shared content between AccountMosaicRestrictionTransaction and EmbeddedAccountMosaicRestrictionTransaction.
inline struct AccountMosaicRestrictionTransactionBody
	# Type of restriction being applied to the listed mosaics.
	restriction_flags = AccountRestrictionFlags

	# Number of mosaics being added.
	restriction_additions_count = uint8

	# Number of mosaics being removed.
	restriction_deletions_count = uint8

	# Reserved padding to align restriction_additions to an 8-byte boundary.
	account_restriction_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# Array of mosaics being added to the restricted list.
	restriction_additions = array(UnresolvedMosaicId, restriction_additions_count)

	# Array of mosaics being removed from the restricted list.
	restriction_deletions = array(UnresolvedMosaicId, restriction_deletions_count)

# Allow or block incoming transactions containing a given set of mosaics (V1, latest).
struct AccountMosaicRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_MOSAIC_RESTRICTION)

	inline Transaction
	inline AccountMosaicRestrictionTransactionBody

# Embedded version of AccountMosaicRestrictionTransaction (V1, latest).
struct EmbeddedAccountMosaicRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_MOSAIC_RESTRICTION)

	inline EmbeddedTransaction
	inline AccountMosaicRestrictionTransactionBody
