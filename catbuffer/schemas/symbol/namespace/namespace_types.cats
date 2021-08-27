using NamespaceId = uint64

# enumeration of namespace registration types
enum NamespaceRegistrationType : uint8
	# root namespace
	root = 0x00

	# child namespace
	child = 0x01

# enumeration of alias actions
enum AliasAction : uint8
	# unlink alias
	unlink = 0x00

	# link alias
	link = 0x01
