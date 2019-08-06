import "namespace/namespace_types.cats"

# binary layout for namespace lifetime
struct NamespaceLifetime
	# lifetime start
	lifetimeStart = Height

	# lifetime end
	lifetimeEnd = Height


# binary layout for a namespace path
struct NamespacePath
	# number of paths 
	pathSize = int64

	# namespace path
	paths = array(NamespaceId, pathSize)


# binary layout for sub-namespace child paths
struct ChildPaths 
	# max child depth
	childMaxDepth = int8

	# child paths
	childPaths = array(NamespacePath, size=childMaxDepth)	


# namespace alias type 
enum AliasType : uint8
	# if alias is mosaicId
	mosaicId = 0

	# if alias is address
	address = 1
