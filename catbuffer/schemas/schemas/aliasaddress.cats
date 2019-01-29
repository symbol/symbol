import "namespacetypes.cats"
import "transaction.cats"

# binary layout for an alias address transaction
struct AliasAddressTransactionBody
	# alias action
	aliasAction = AliasAction

	# id of a namespace that will become an alias
	namespaceId = NamespaceId

	# aliased address
	address = Address

# binary layout for a non-embedded alias address transaction
struct AliasAddressTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424E

	inline Transaction
	inline AliasAddressTransactionBody

# binary layout for an embedded alias address transaction
struct EmbeddedAliasAddressTransaction
	inline EmbeddedTransaction
	inline AliasAddressTransactionBody
