using NamespaceId = uint64

# namespace types
enum NamespaceType : uint8
	# a root namespace
	root = 0x00

	# a child namespace
	child = 0x01

# alias transaction action
enum AliasAction : uint8
	# link alias
	link = 0x00

	# unlink alias
	unlink = 0x01
