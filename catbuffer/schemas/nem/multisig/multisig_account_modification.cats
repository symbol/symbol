import "transaction.cats"

# enumeration of multisig account modification types
enum MultisigAccountModificationType : uint32
	# add a new cosignatory
	ADD_COSIGNATORY = 1

	# delete an existing cosignatory
	DELETE_COSIGNATORY = 2

# binary layout for a multisig account modification
struct MultisigAccountModification
	# modification type
	modification_type = MultisigAccountModificationType

	# [__value__] cosignatory public key
	#
	# [size] cosignatory public size
	cosignatory_public_key = inline SizePrefixedPublicKey

# binary layout for a multisig account modification transaction (V1)
struct MultisigAccountModificationTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	inline Transaction

	# number of modifications
	modifications_count = uint32

	# multisig account modifications
	modifications = array(MultisigAccountModification, modifications_count)

# binary layout for a multisig account modification transaction (V2)
struct MultisigAccountModificationTransaction2
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline MultisigAccountModificationTransaction

	# relative change of the minimal number of cosignatories required when approving a transaction
	min_approval_delta = int32
