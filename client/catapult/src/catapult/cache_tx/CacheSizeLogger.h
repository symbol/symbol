/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/utils/FileSize.h"

namespace catapult { namespace cache {

	/// Logs cache sizes for a cache with \a name at specified levels of fullness
	/// given its old (\a oldSize), new (\a newSize) and max (\a maxSize) sizes.
	inline void LogSizes(const char* name, utils::FileSize oldSize, utils::FileSize newSize, utils::FileSize maxSize) {
		auto logCacheSizeIf = [name, oldSize, newSize, maxSize](uint32_t percentage, const auto* description) {
			auto desiredSize = utils::FileSize::FromBytes(maxSize.bytes() * percentage / 100);
			if (oldSize > desiredSize || newSize < desiredSize)
				return;

			CATAPULT_LOG(warning) << name << " cache is " << description << " (size = " << newSize << ")";
		};

		// log if cache is filling up
		logCacheSizeIf(50, "half full");
		logCacheSizeIf(90, "90 percent full");
		logCacheSizeIf(95, "95 percent full");
		logCacheSizeIf(99, "99 percent full");
	}
}}
