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

	/// Number of buffer values required to guarantee deterministic rollbacks.
	constexpr size_t Rollback_Buffer_Size = 2;

	/// Number of historical importances associated with a single account.
	constexpr size_t Importance_History_Size = 1 + Rollback_Buffer_Size;

	/// Number of historical activity buckets associated with a single account.
	/// \note This allows four full buckets and one partial (WIP) bucket.
	constexpr size_t Activity_Bucket_History_Size = 5 + Rollback_Buffer_Size;

	/// Size of hashes in the hash cache.
	/// \note Reducing below `Hash256::Size` can save memory but will increase possibility of false positive rejections.
	constexpr size_t Cached_Hash_Size = Hash256::Size;

	/// Duration of eternal artifact.
	constexpr BlockDuration Eternal_Artifact_Duration(0);

	/// Number of files per storage subdirectory.
	constexpr size_t Files_Per_Storage_Directory = 10'000;

	/// Default maximum packet data size when not explicitly specified.
	constexpr uint32_t Default_Max_Packet_Data_Size = 100 * 1024 * 1024;
}
