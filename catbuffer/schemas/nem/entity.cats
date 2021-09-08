import "types.cats"

# enumeration of network types
enum NetworkType : uint16
	# main network
	MAINNET = 0x68

	# test network
	TESTNET = 0x98

# binary layout for a blockchain entity (block or transaction)
struct EntityBody
	# entity version
	version = uint16

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
