import "transaction.cats"

# cosignatory modification type
enum CosignatoryModificationType : uint8
	# add cosignatory
	add = 0

	# remove cosignatory
	del = 1

# binary layout for a cosignatory modification
struct CosignatoryModification
	# modification type
	modificationType = CosignatoryModificationType

	# cosignatory account public key
	cosignatoryPublicKey = Key

# binary layout for a modify multisig account transaction
struct ModifyMultisigAccountTransactionBody
	# relative change of the minimal number of cosignatories required when removing an account
	minRemovalDelta = int8

	# relative change of the minimal number of cosignatories required when approving a transaction
	minApprovalDelta = int8

	# number of modifications
	modificationsCount = uint8

	# attached modifications
	modifications = array(CosignatoryModification, modificationsCount)

# binary layout for a non-embedded modify multisig account transaction
struct ModifyMultisigAccountTransaction
	const uint8 version = 3
	const EntityType entityType = 0x4155

	inline Transaction
	inline ModifyMultisigAccountTransactionBody

# binary layout for an embedded modify multisig account transaction
struct EmbeddedModifyMultisigAccountTransaction
	inline EmbeddedTransaction
	inline ModifyMultisigAccountTransactionBody
