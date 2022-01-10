import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# Shared content between AccountAddressRestrictionTransaction and EmbeddedAccountAddressRestrictionTransaction.
inline struct AccountAddressRestrictionTransactionBody
	# Type of restriction being applied to the listed addresses.
	restriction_flags = AccountRestrictionFlags

	# Number of addresses being added.
	restriction_additions_count = uint8

	# Number of addresses being removed.
	restriction_deletions_count = uint8

	# Reserved padding to align restriction_additions to an 8-byte boundary.
	account_restriction_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# Array of account addresses being added to the restricted list.
	restriction_additions = array(UnresolvedAddress, restriction_additions_count)

	# Array of account addresses being removed from the restricted list.
	restriction_deletions = array(UnresolvedAddress, restriction_deletions_count)

# Allow or block incoming and outgoing transactions for a given a set of addresses.
struct AccountAddressRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_ADDRESS_RESTRICTION)

	inline Transaction
	inline AccountAddressRestrictionTransactionBody

# Embedded version of AccountAddressRestrictionTransaction.
struct EmbeddedAccountAddressRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_ADDRESS_RESTRICTION)

	inline EmbeddedTransaction
	inline AccountAddressRestrictionTransactionBody
