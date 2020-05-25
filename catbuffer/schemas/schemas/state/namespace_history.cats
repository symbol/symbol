import "mosaic/mosaic_definition.cats"
import "state/namespace_history_types.cats"

# binary layout for non-historical root namespace history
struct RootNamespaceHistory
	# id of the root namespace history
	id = NamespaceId

	# namespace owner address
	ownerAddress = Address

	# lifetime in blocks
	lifetime = NamespaceLifetime

	# root namespace alias
	rootAlias = NamespaceAliasType

	# number of children
	childrenCount = uint64

	# save child sub-namespace paths
	paths = array(NamespacePath, childrenCount, sort_key=path)
