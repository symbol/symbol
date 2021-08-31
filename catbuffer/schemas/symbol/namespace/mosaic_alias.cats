import "namespace/namespace_types.cats"
import "transaction.cats"

# binary layout for an mosaic alias transaction
struct MosaicAliasTransactionBody
	# identifier of the namespace that will become an alias
	namespaceId = NamespaceId

	# aliased mosaic identifier
	mosaicId = MosaicId

	# alias action
	aliasAction = AliasAction

# binary layout for a non-embedded mosaic alias transaction
struct MosaicAliasTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_alias)

	inline Transaction
	inline MosaicAliasTransactionBody

# binary layout for an embedded mosaic alias transaction
struct EmbeddedMosaicAliasTransaction
	transaction_version = make_const(uint8, 1)
	transaction_type = make_const(TransactionType, mosaic_alias)

	inline EmbeddedTransaction
	inline MosaicAliasTransactionBody
