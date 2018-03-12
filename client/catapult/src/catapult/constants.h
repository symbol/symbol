#pragma once
#include "types.h"

namespace catapult {

	/// The number of historical importances.
	constexpr size_t Importance_History_Size = 3;

	/// The size of hashes in the hash cache.
	constexpr size_t Cached_Hash_Size = Hash256_Size;

	/// The XEM mosaic id.
#ifdef SIGNATURE_SCHEME_NIS1
	constexpr MosaicId Xem_Id = MosaicId(0x493B6D561C49F4B8ULL);
#else
	constexpr MosaicId Xem_Id = MosaicId(0xD525AD41D95FCF29ULL);
#endif
}
