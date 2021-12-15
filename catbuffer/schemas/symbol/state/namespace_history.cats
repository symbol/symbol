import "state/namespace_history_types.cats"
import "state/state_header.cats"

# binary layout for non-historical root namespace history
struct RootNamespaceHistory
	inline StateHeader

	# id of the root namespace history
	id = NamespaceId

	# namespace owner address
	owner_address = Address

	# lifetime in blocks
	lifetime = NamespaceLifetime

	# root namespace alias
	root_alias = NamespaceAlias

	# number of children
	children_count = uint64

	# save child sub-namespace paths
	@sort_key(path)
	paths = array(NamespacePath, children_count)
