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
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, address_alias)

	inline Transaction
	inline AddressAliasTransactionBody

# binary layout for an embedded address alias transaction
struct EmbeddedAddressAliasTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, address_alias)

	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
