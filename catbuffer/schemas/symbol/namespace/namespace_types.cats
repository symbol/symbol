using NamespaceId = uint64

# Enumeration of namespace registration types.
enum NamespaceRegistrationType : uint8
	# Root namespace.
	ROOT = 0x00

	# Child namespace.
	CHILD = 0x01

# Enumeration of alias actions.
enum AliasAction : uint8
	# Unlink a namespace, removing the alias.
	UNLINK = 0x00

	# Link a namespace, creating an alias.
	LINK = 0x01
