using NamespaceId = uint64

# namespace types
enum NamespaceType : uint8
	# a root namespace
	root = 0

	# a child namespace
	child = 1

# alias transaction action
enum AliasAction : uint8
	# link alias
	link = 0

	# unlink alias
	unlink = 1
