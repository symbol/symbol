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
	const uint8 transaction_version = 1
	const TransactionType transaction_type = address_alias

	inline Transaction
	inline AddressAliasTransactionBody

# binary layout for an embedded address alias transaction
struct EmbeddedAddressAliasTransaction
	const uint8 transaction_version = 1
	const TransactionType transaction_type = address_alias

	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
