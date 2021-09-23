import "types.cats"

# enumeration of network types
enum NetworkType : uint8
	# main network
	MAINNET = 0x68

	# test network
	TESTNET = 0x98

# binary layout for a blockchain entity (block or transaction)
struct EntityBody
	# entity version
	version = uint8

	# reserved padding between version and network type
	entity_body_reserved_1 = make_reserved(uint16, 0)

	# entity network
	network = NetworkType

	# entity timestamp
	timestamp = Timestamp

	# [__value__] entity signer public key
	#
	# [size] entity signer public key size
	signer_public_key = inline SizePrefixedPublicKey

	# [__value__] entity signature
	#
	# [size] entity signature size
	signature = inline SizePrefixedSignature
