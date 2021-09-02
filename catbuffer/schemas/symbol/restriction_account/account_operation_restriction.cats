import "restriction_account/restriction_account_types.cats"
import "transaction.cats"

# binary layout for an account operation restriction transaction
struct AccountOperationRestrictionTransactionBody
	# account restriction flags
	restriction_flags = AccountRestrictionFlags

	# number of account restriction additions
	restriction_additions_count = uint8

	# number of account restriction deletions
	restriction_deletions_count = uint8

	# reserved padding to align restrictionAdditions on 8-byte boundary
	account_restriction_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# account restriction additions
	restriction_additions = array(TransactionType, restriction_additions_count)

	# account restriction deletions
	restriction_deletions = array(TransactionType, restriction_deletions_count)

# binary layout for a non-embedded account operation restriction transaction
struct AccountOperationRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_OPERATION_RESTRICTION)

	inline Transaction
	inline AccountOperationRestrictionTransactionBody

# binary layout for an embedded account operation restriction transaction
struct EmbeddedAccountOperationRestrictionTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ACCOUNT_OPERATION_RESTRICTION)

	inline EmbeddedTransaction
	inline AccountOperationRestrictionTransactionBody
