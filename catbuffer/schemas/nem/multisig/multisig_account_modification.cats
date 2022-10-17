import "transaction.cats"

# enumeration of multisig account modification types
enum MultisigAccountModificationType : uint32
	# add a new cosignatory
	ADD_COSIGNATORY = 1

	# delete an existing cosignatory
	DELETE_COSIGNATORY = 2

# binary layout for a multisig account modification
@is_size_implicit
@comparer(modification_type, cosignatory_public_key!ripemd_keccak_256)
struct MultisigAccountModification
	# modification type
	modification_type = MultisigAccountModificationType

	# cosignatory public key size
	cosignatory_public_key_size = make_reserved(uint32, 32)

	# cosignatory public key
	cosignatory_public_key = PublicKey

# binary layout for a multisig account modification prefixed with size
struct SizePrefixedMultisigAccountModification
	# modification size
	modification_size = sizeof(uint32, modification)

	# modification
	modification = MultisigAccountModification

# shared content between all verifiable and non-verifiable multisig account transactions
inline struct MultisigAccountModificationTransactionBody
	TRANSACTION_TYPE = make_const(TransactionType, MULTISIG_ACCOUNT_MODIFICATION)

	# number of modifications
	modifications_count = uint32

	# multisig account modifications
	@sort_key(modification)
	modifications = array(SizePrefixedMultisigAccountModification, modifications_count)

# shared content between V1 verifiable and non-verifiable multisig account transactions
inline struct MultisigAccountModificationTransactionV1Body
	TRANSACTION_VERSION = make_const(uint8, 1)

	inline MultisigAccountModificationTransactionBody

# shared content between V2 verifiable and non-verifiable multisig account transactions
inline struct MultisigAccountModificationTransactionV2Body
	TRANSACTION_VERSION = make_const(uint8, 2)

	inline MultisigAccountModificationTransactionBody

	# the size of the min_approval_delta
	min_approval_delta_size = make_reserved(uint32, 4)

	# relative change of the minimal number of cosignatories required when approving a transaction
	min_approval_delta = int32

# binary layout for a multisig account modification transaction (V1)
struct MultisigAccountModificationTransactionV1
	inline Transaction
	inline MultisigAccountModificationTransactionV1Body

# binary layout for a non-verifiable multisig account modification transaction (V1)
struct NonVerifiableMultisigAccountModificationTransactionV1
	inline NonVerifiableTransaction
	inline MultisigAccountModificationTransactionV1Body

# binary layout for a multisig account modification transaction (V2, latest)
struct MultisigAccountModificationTransactionV2
	inline Transaction
	inline MultisigAccountModificationTransactionV2Body

# binary layout for a non-verifiable multisig account modification transaction (V2, latest)
struct NonVerifiableMultisigAccountModificationTransactionV2
	inline NonVerifiableTransaction
	inline MultisigAccountModificationTransactionV2Body
