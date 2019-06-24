import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for a register namespace transaction
struct RegisterNamespaceTransactionBody
	# type of the registered namespace
	namespaceType = NamespaceType

	# namespace duration
	duration = BlockDuration if namespaceType equals root

	# id of the parent namespace
	parentId = NamespaceId if namespaceType equals child

	# id of the namespace
	namespaceId = NamespaceId

	# size of the namespace name
	namespaceNameSize = uint8

	# namespace name
	name = array(byte, namespaceNameSize)

# binary layout for a non-embedded register namespace transaction
struct RegisterNamespaceTransaction
	const uint8 version = 1
	const EntityType entityType = 0x414E

	inline Transaction
	inline RegisterNamespaceTransactionBody

# binary layout for an embedded register namespace transaction
struct EmbeddedRegisterNamespaceTransaction
	inline EmbeddedTransaction
	inline RegisterNamespaceTransactionBody

