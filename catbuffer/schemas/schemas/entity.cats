import "types.cats"

# enumeration of entity types
enum EntityType : uint16
	# reserved entity type
	reserved = 0x0000

# binary layout for a size-prefixed entity
struct SizePrefixedEntity
	# entity size
	size = uint32

# binary layout for a verifiable entity
struct VerifiableEntity
	# entity signature
	signature = Signature

# binary layout for a blockchain entity (block or transaction)
struct EntityBody
	# entity signer's public key
	signerPublicKey = Key

	# entity version
	version = uint16

	# entity type
	type = EntityType
