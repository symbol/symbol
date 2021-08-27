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
	const uint8 version = 1
	const EntityType entityType = 0x434E

	inline Transaction
	inline MosaicAliasTransactionBody

# binary layout for an embedded mosaic alias transaction
struct EmbeddedMosaicAliasTransaction
	const uint8 version = 1
	const EntityType entityType = 0x434E

	inline EmbeddedTransaction
	inline MosaicAliasTransactionBody
