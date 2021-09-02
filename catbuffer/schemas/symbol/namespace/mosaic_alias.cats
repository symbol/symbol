import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for an mosaic alias transaction
struct MosaicAliasTransactionBody
	# identifier of the namespace that will become an alias
	namespace_id = NamespaceId

	# aliased mosaic identifier
	mosaic_id = MosaicId

	# alias action
	alias_action = AliasAction

# binary layout for a non-embedded mosaic alias transaction
struct MosaicAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ALIAS)

	inline Transaction
	inline MosaicAliasTransactionBody

# binary layout for an embedded mosaic alias transaction
struct EmbeddedMosaicAliasTransaction
	TRANSACTION_VERSION = make_const(uint8, 1)
	TRANSACTION_TYPE = make_const(TransactionType, MOSAIC_ALIAS)

	inline EmbeddedTransaction
	inline MosaicAliasTransactionBody
