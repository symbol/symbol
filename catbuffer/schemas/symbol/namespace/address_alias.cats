import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for an address alias transaction
struct AddressAliasTransactionBody
	# identifier of the namespace that will become an alias
	namespace_id = NamespaceId

	# aliased address
	address = Address

	# alias action
	alias_action = AliasAction

# binary layout for a non-embedded address alias transaction
struct AddressAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ADDRESS_ALIAS)

	inline Transaction
	inline AddressAliasTransactionBody

# binary layout for an embedded address alias transaction
struct EmbeddedAddressAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ADDRESS_ALIAS)

	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
