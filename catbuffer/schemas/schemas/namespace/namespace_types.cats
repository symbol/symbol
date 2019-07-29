using NamespaceId = uint64

# namespace registration type
enum NamespaceRegistrationType : uint8
	# root namespace
	root = 0x00

	# child namespace
	child = 0x01

# alias transaction action
enum AliasAction : uint8
	# unlink alias
	unlink = 0x00

	# link alias
	link = 0x01
