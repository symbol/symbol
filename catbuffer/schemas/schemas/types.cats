using Amount = uint64
using BlockDuration = uint64
using UnresolvedMosaicId = uint64
using MosaicId = uint64
using Timestamp = uint64

using UnresolvedAddress = binary_fixed(25)
using Address = binary_fixed(25)
using Hash256 = binary_fixed(32)
using Hash512 = binary_fixed(64)
using Key = binary_fixed(32)
using Signature = binary_fixed(64)

# binary layout for a mosaic
struct Mosaic
	# mosaic identifier
	mosaicId = MosaicId
	# mosaic amount
	amount = Amount

# binary layout for an unresolved mosaic
struct UnresolvedMosaic
	# mosaic identifier
	mosaicId = UnresolvedMosaicId
	# mosaic amount
	amount = Amount
