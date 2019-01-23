import "namespacetypes.cats"
import "transaction.cats"

# binary layout for an alias mosaic transaction
struct AliasMosaicTransactionBody
	# alias transaction action
	aliasAction = AliasAction

	# id of a namespace that will become an alias
	namespaceId = NamespaceId

	# aliased mosaic id
	mosaicId = MosaicId

# binary layout for a non-embedded alias mosaic transaction
struct AliasMosaicTransaction
	const uint8 version = 1
	const EntityType entityType = 0x434E

	inline Transaction
	inline AliasMosaicTransactionBody

# binary layout for an embedded alias mosaic transaction
struct EmbeddedAliasMosaicTransaction
	inline EmbeddedTransaction
	inline AliasMosaicTransactionBody
