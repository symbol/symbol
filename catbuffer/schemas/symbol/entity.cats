import "types.cats"

# enumeration of entity types
enum EntityType : uint16
	# reserved entity type
	reserved = 0x0000

# enumeration of network types
enum NetworkType : uint8
	# mijin network
	mijin = 0x60
	
	# public network
	public = 0x68

	# private network
	private = 0x78
	
	# mijin test network
	mijinTest = 0x90

	# public test network
	publicTest = 0x98

	# private test network
	privateTest = 0xA8

# binary layout for a size-prefixed entity
struct SizePrefixedEntity
	# entity size
	size = uint32

# binary layout for a verifiable entity
struct VerifiableEntity
	# reserved padding to align Signature on 8-byte boundary
	verifiableEntityHeader_Reserved1 = uint32

	# entity signature
	signature = Signature

# binary layout for a blockchain entity (block or transaction)
struct EntityBody
	# entity signer's public key
	signerPublicKey = Key

	# reserved padding to align end of EntityBody on 8-byte boundary
	entityBody_Reserved1 = uint32

	# entity version
	version = uint8

	# entity network
	network = NetworkType

	# entity type
	type = EntityType
