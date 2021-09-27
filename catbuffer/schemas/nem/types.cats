using Amount = uint64
using Height = uint64
using Timestamp = uint32

using Address = binary_fixed(40)
using Hash256 = binary_fixed(32)
using PublicKey = binary_fixed(32)
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
