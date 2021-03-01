import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for an address alias transaction
struct AddressAliasTransactionBody
	# identifier of the namespace that will become an alias
	namespaceId = NamespaceId

	# aliased address
	address = Address

	# alias action
	aliasAction = AliasAction

# binary layout for a non-embedded address alias transaction
struct AddressAliasTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424E

	inline Transaction
	inline AddressAliasTransactionBody

# binary layout for an embedded address alias transaction
struct EmbeddedAddressAliasTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424E

	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
