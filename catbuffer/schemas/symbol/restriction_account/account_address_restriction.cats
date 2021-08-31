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
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_address_restriction)

	inline Transaction
	inline AccountAddressRestrictionTransactionBody

# binary layout for an embedded account address restriction transaction
struct EmbeddedAccountAddressRestrictionTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, account_address_restriction)

	inline EmbeddedTransaction
	inline AccountAddressRestrictionTransactionBody
