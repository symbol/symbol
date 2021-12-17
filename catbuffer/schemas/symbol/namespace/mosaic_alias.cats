import "namespace/namespace_types.cats"
import "transaction.cats"

# Shared content between MosaicAliasTransaction and EmbeddedMosaicAliasTransaction
inline struct MosaicAliasTransactionBody
	# Identifier of the namespace that will become (or stop being) an alias for the Mosaic.
	namespace_id = NamespaceId

	# Aliased mosaic identifier.
	mosaic_id = MosaicId

	# Alias action.
	alias_action = AliasAction

# Attach or detach a [namespace](/concepts/namespace.html) to a Mosaic.
#
# Setting an alias to a mosaic is only possible if the account announcing this transaction
# has also created the namespace and the mosaic involved.
struct MosaicAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ALIAS)

	inline Transaction
	inline MosaicAliasTransactionBody

# Embedded version of MosaicAliasTransaction
struct EmbeddedMosaicAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ALIAS)

	inline EmbeddedTransaction
	inline MosaicAliasTransactionBody
