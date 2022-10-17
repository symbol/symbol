import "namespace/namespace_types.cats"
import "transaction.cats"

# Shared content between AddressAliasTransaction and EmbeddedAddressAliasTransaction.
inline struct AddressAliasTransactionBody
	# Identifier of the namespace that will become (or stop being) an alias for the address.
	namespace_id = NamespaceId

	# Aliased address.
	address = Address

	# Alias action.
	alias_action = AliasAction

# Attach or detach a [namespace](/concepts/namespace.html) (alias) to an account address (V1, latest).
#
# A namespace can be assigned to any account present in the network (this is, an account
# which has received at least one transaction).
struct AddressAliasTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ADDRESS_ALIAS)

	inline Transaction
	inline AddressAliasTransactionBody

# Embedded version of AddressAliasTransaction (V1, latest).
struct EmbeddedAddressAliasTransactionV1
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, ADDRESS_ALIAS)

	inline EmbeddedTransaction
	inline AddressAliasTransactionBody
