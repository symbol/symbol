import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# Shared content between AccountOperationRestrictionTransaction and EmbeddedAccountOperationRestrictionTransaction.
inline struct AccountOperationRestrictionTransactionBody
	# Type of restriction being applied to the listed transaction types.
	restriction_flags = AccountRestrictionFlags

	# Number of transaction types being added.
	restriction_additions_count = uint8

	# Number of transaction types being removed.
	restriction_deletions_count = uint8

	# Reserved padding to align restriction_additions to an 8-byte boundary.
	account_restriction_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# Array of transaction types being added to the restricted list.
	restriction_additions = array(TransactionType, restriction_additions_count)

	# Array of transaction types being rtemoved from the restricted list.
	restriction_deletions = array(TransactionType, restriction_deletions_count)

# Allow or block outgoing transactions depending on their transaction type (V1, latest).
struct AccountOperationRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_OPERATION_RESTRICTION)

	inline Transaction
	inline AccountOperationRestrictionTransactionBody

# Embedded version of AccountOperationRestrictionTransaction (V1, latest).
struct EmbeddedAccountOperationRestrictionTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_OPERATION_RESTRICTION)

	inline EmbeddedTransaction
	inline AccountOperationRestrictionTransactionBody
