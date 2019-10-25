import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a namespace registration transaction
struct NamespaceRegistrationTransactionBody
	# namespace duration
	duration = BlockDuration if registrationType equals root

	# parent namespace identifier
	parentId = NamespaceId if registrationType equals child

	# namespace identifier
	id = NamespaceId

	# namespace registration type
	registrationType = NamespaceRegistrationType

	# namespace name size
	nameSize = uint8

	# namespace name
	name = array(byte, nameSize)

# binary layout for a non-embedded namespace registration transaction
struct NamespaceRegistrationTransaction
	const uint8 version = 1
	const EntityType entityType = 0x414E

	inline Transaction
	inline NamespaceRegistrationTransactionBody

# binary layout for an embedded namespace registration transaction
struct EmbeddedNamespaceRegistrationTransaction
	inline EmbeddedTransaction
	inline NamespaceRegistrationTransactionBody
