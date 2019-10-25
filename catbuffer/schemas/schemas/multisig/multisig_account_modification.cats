import "transaction.cats"

# binary layout for a multisig account modification transaction
struct MultisigAccountModificationTransactionBody
	# relative change of the minimal number of cosignatories required when removing an account
	minRemovalDelta = int8

	# relative change of the minimal number of cosignatories required when approving a transaction
	minApprovalDelta = int8

	# number of cosignatory public key additions
	publicKeyAdditionsCount = uint8

	# number of cosignatory public key deletions
	publicKeyDeletionsCount = uint8

	# reserved padding to align publicKeyAdditions on 8-byte boundary
	multisigAccountModificationTransactionBody_Reserved1 = uint32

	# cosignatory public key additions
	publicKeyAdditions = array(Key, publicKeyAdditionsCount)

	# cosignatory public key deletions
	publicKeyDeletions = array(Key, publicKeyDeletionsCount)

# binary layout for a non-embedded multisig account modification transaction
struct MultisigAccountModificationTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4155

	inline Transaction
	inline MultisigAccountModificationTransactionBody

# binary layout for an embedded multisig account modification transaction
struct EmbeddedMultisigAccountModificationTransaction
	inline EmbeddedTransaction
	inline MultisigAccountModificationTransactionBody
