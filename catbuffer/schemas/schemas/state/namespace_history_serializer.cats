import "mosaic/mosaic_definition.cats"
import "state/namespace_history_serializer_types.cats"

# binary layout for non-historical root namespace history serializer
struct RootNamespaceHistory

	# parent namespace owner
	owner = Key

	# parent namespace id 
	namespaceId = NamespaceId

	# alias type
	aliasType = AliasType

	# lifetime in blocks
	lifetime = NamespaceLifetime

	# if mosaic id alias, assign AliasType as MosaicId
	mosaicAlias = MosaicId if aliasType equals mosaicId

	# if address alias, assign AliasType as Address
	addressAlias = Address if aliasType equals address

	# save child sub-namespace paths
	childPaths = ChildPaths
