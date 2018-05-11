/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "types.h"

namespace catapult {

	/// Number of historical importances associated with a single account.
	constexpr size_t Importance_History_Size = 3;

	/// Size of hashes in the hash cache.
	/// \note Reducing below `Hash256_Size` can save memory but will increase possibility of false positive rejections.
	constexpr size_t Cached_Hash_Size = Hash256_Size;

	/// XEM mosaic id.
#ifdef SIGNATURE_SCHEME_NIS1
	constexpr MosaicId Xem_Id = MosaicId(0x493B6D561C49F4B8ULL);
#else
	constexpr MosaicId Xem_Id = MosaicId(0xD525AD41D95FCF29ULL);
#endif
}
