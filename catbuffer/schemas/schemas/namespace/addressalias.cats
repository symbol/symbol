import "namespace/namespacetypes.cats"
import "transaction.cats"

# binary layout for an address alias transaction
struct AddressAliasTransactionBody
	# alias action
	aliasAction = AliasAction

	# id of a namespace that will become an alias
	namespaceId = NamespaceId

	# aliased address
	address = Address

# binary layout for a non-embedded address alias transaction
struct AddressAliasTransaction
	const uint8 version = 1
	const EntityType entityType = 0x424E

	inline Transaction
	inline AddressAliasTransactionBody

# binary layout for an embedded address alias transaction
struct EmbeddedAddressAliasTransaction
	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
