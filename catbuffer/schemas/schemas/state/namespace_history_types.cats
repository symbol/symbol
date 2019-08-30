import "namespace/namespace_types.cats"
import "types.cats"

# binary layout for namespace lifetime
struct NamespaceLifetime
	# start height
	lifetimeStart = Height

	# end height
	lifetimeEnd = Height

# namespace alias type
enum NamespaceAliasType : uint8
	# no alias
	none = 0

	# if alias is mosaicId
	mosaicId = 1

	# if alias is address
	address = 2

# binary layout for alias
struct NamespaceAlias
	# namespace alias type
	namespaceAliasType = NamespaceAliasType

	# mosaic alias
	mosaicAlias = MosaicId if namespaceAliasType equals mosaicId

	# address alias
	addressAlias = Address if namespaceAliasType equals address

# binary layout for a namespace path
struct NamespacePath
	# number of paths (excluding root id)
	pathSize = uint64

	# namespace path (excluding root id)
	path = array(NamespaceId, pathSize)

	# namespace alias
	alias = NamespaceAlias
