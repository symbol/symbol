import "transaction.cats"

# binary layout for a multisig account modification transaction
struct MultisigAccountModificationTransactionBody
	# relative change of the minimal number of cosignatories required when removing an account
	minRemovalDelta = int8

	# relative change of the minimal number of cosignatories required when approving a transaction
	minApprovalDelta = int8

	# number of cosignatory address additions
	addressAdditionsCount = uint8

	# number of cosignatory address deletions
	addressDeletionsCount = uint8

	# reserved padding to align addressAdditions on 8-byte boundary
	multisigAccountModificationTransactionBody_Reserved1 = uint32

	# cosignatory address additions
	addressAdditions = array(UnresolvedAddress, addressAdditionsCount)

	# cosignatory address deletions
	addressDeletions = array(UnresolvedAddress, addressDeletionsCount)

# binary layout for a non-embedded multisig account modification transaction
struct MultisigAccountModificationTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4155

	inline Transaction
	inline MultisigAccountModificationTransactionBody

# binary layout for an embedded multisig account modification transaction
struct EmbeddedMultisigAccountModificationTransaction
	const uint8 version = 1
	const EntityType entityType = 0x4155

	inline EmbeddedTransaction
	inline MultisigAccountModificationTransactionBody
