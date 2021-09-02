import "types.cats"

# enumeration of network types
enum NetworkType : uint8
	# public network
	MAINNET = 0x68

	# public test network
	TESTNET = 0x98

# binary layout for a size-prefixed entity
struct SizePrefixedEntity
	# entity size
	size = uint32

# binary layout for a verifiable entity
struct VerifiableEntity
	# reserved padding to align Signature on 8-byte boundary
	verifiable_entity_header_reserved_1 = make_reserved(uint32, 0)

	# entity signature
	signature = Signature

# binary layout for a blockchain entity (block or transaction)
struct EntityBody
	# entity signer's public key
	signer_public_key = PublicKey

	# reserved padding to align end of EntityBody on 8-byte boundary
	entity_body_reserved_1 = make_reserved(uint32, 0)

	# entity version
	version = uint8

	# entity network
	network = NetworkType
