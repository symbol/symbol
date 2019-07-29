import "multisig/multisig_types.cats"
import "transaction.cats"

# binary layout for a multisig account modification transaction
struct MultisigAccountModificationTransactionBody
	# relative change of the minimal number of cosignatories required when removing an account
	minRemovalDelta = int8

	# relative change of the minimal number of cosignatories required when approving a transaction
	minApprovalDelta = int8

	# number of modifications
	modificationsCount = uint8

	# attached cosignatory modifications
	modifications = array(CosignatoryModification, modificationsCount)

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
