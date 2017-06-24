#pragma once
#include "catapult/types.h"

namespace catapult {

	/// The number of historical importances.
	constexpr size_t Importance_History_Size = 3;

	/// The size of hashes in the hash cache.
	constexpr size_t Cached_Hash_Size = Hash256_Size;

#ifdef NIS1_COMPATIBLE_SIGNATURES
	constexpr MosaicId Xem_Id = MosaicId(0xcb1804395a65a504ULL);
#else
	constexpr MosaicId Xem_Id = MosaicId(0xD525AD41D95FCF29ULL);
#endif
}
