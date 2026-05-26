# A quantity of mosaics in absolute units.
# It can only be positive or zero.
using Amount = uint64

# Index of a block in the blockchain.
# The first block (the Nemesis block) has height 1 and each subsequent block increases height by 1.
using Height = uint64

# Number of seconds elapsed since the creation of the Nemesis block.
using Timestamp = uint32

# An address identifies an account and is derived from its PublicKey. The 40 bytes correspond to its Base32-encoded form.
using Address = binary_fixed(40)

# A 32-byte (256 bit) hash.
# The exact algorithm is unspecified as it can change depending on where it is used.
using Hash256 = binary_fixed(32)

# A 32-byte (256 bit) integer derived from a private key.
# It serves as the public identifier of the key pair and can be disseminated widely. It is used to prove that an entity was signed with the paired private key.
using PublicKey = binary_fixed(32)

# A 64-byte (512 bit) array certifying that the signed data has not been modified.
# NEM uses Ed25519 signatures with the Keccak-512 hash function.
using Signature = binary_fixed(64)

# binary layout for a size prefixed address
inline struct SizePrefixedAddress
	# address size
	size = make_reserved(uint32, 40)

	# address value
	__value__ = Address

# binary layout for a size prefixed 32-byte hash
inline struct SizePrefixedHash256
	# hash size
	size = make_reserved(uint32, 32)

	# hash value
	__value__ = Hash256

# binary layout for a size prefixed public key
inline struct SizePrefixedPublicKey
	# public key size
	size = make_reserved(uint32, 32)

	# public key value
	__value__ = PublicKey

# binary layout for a size prefixed signature
inline struct SizePrefixedSignature
	# signature size
	size = make_reserved(uint32, 64)

	# signature value
	__value__ = Signature

# binary layout for a size prefixed string
inline struct SizePrefixedString
	# string size
	size = uint32

	# string value
	__value__ = array(int8, size)
