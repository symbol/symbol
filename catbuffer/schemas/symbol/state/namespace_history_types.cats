import "namespace/namespace_types.cats"
import "types.cats"

# binary layout for namespace lifetime
struct NamespaceLifetime
	# start height
	lifetime_start = Height

	# end height
	lifetime_end = Height

# namespace alias type
enum NamespaceAliasType : uint8
	# no alias
	NONE = 0

	# if alias is mosaicId
	MOSAIC_ID = 1

	# if alias is address
	ADDRESS = 2

# binary layout for alias
struct NamespaceAlias
	# namespace alias type
	namespace_alias_type = NamespaceAliasType

	# mosaic alias
	mosaic_alias = MosaicId if MOSAIC_ID equals namespace_alias_type

	# address alias
	address_alias = Address if ADDRESS equals namespace_alias_type

# binary layout for a namespace path
struct NamespacePath
	# number of paths (excluding root id)
	path_size = uint8

	# namespace path (excluding root id)
	path = array(NamespaceId, path_size)

	# namespace alias
	alias = NamespaceAlias
