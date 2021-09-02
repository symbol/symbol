using NamespaceId = uint64

# enumeration of namespace registration types
enum NamespaceRegistrationType : uint8
	# root namespace
	ROOT = 0x00

	# child namespace
	CHILD = 0x01

# enumeration of alias actions
enum AliasAction : uint8
	# unlink alias
	UNLINK = 0x00

	# link alias
	LINK = 0x01
