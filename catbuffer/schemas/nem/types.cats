using Amount = uint64
using Timestamp = uint32

using Address = binary_fixed(40)
using PublicKey = binary_fixed(32)

# binary layout for a size prefixed address
inline struct SizePrefixedAddress
	# address size
	size = make_reserved(uint32, 40)

	# address value
	__value__ = Address

# binary layout for a size prefixed public key
inline struct SizePrefixedPublicKey
	# address size
	size = make_reserved(uint32, 32)

	# address value
	__value__ = PublicKey

# binary layout for a size prefixed string
inline struct SizePrefixedString
	# string size
	size = uint32

	# string value
	__value__ = array(int8, size)
