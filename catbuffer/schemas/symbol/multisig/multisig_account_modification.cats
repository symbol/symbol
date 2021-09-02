import "transaction.cats"

# binary layout for a multisig account modification transaction
struct MultisigAccountModificationTransactionBody
	# relative change of the minimal number of cosignatories required when removing an account
	min_removal_delta = int8

	# relative change of the minimal number of cosignatories required when approving a transaction
	min_approval_delta = int8

	# number of cosignatory address additions
	address_additions_count = uint8

	# number of cosignatory address deletions
	address_deletions_count = uint8

	# reserved padding to align addressAdditions on 8-byte boundary
	multisig_account_modification_transaction_body_reserved_1 = make_reserved(uint32, 0)

	# cosignatory address additions
	address_additions = array(UnresolvedAddress, address_additions_count)

	# cosignatory address deletions
	address_deletions = array(UnresolvedAddress, address_deletions_count)

# binary layout for a non-embedded multisig account modification transaction
struct MultisigAccountModificationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	inline Transaction
	inline MultisigAccountModificationTransactionBody

# binary layout for an embedded multisig account modification transaction
struct EmbeddedMultisigAccountModificationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	inline EmbeddedTransaction
	inline MultisigAccountModificationTransactionBody
