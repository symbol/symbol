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
#include <stdint.h>

namespace catapult { namespace cache {

	/// Logs cache sizes for a cache with \a name at specified levels of fullness given its \a actual and \a max sizes.
	inline
	void LogSizes(const char* name, size_t actual, uint64_t max) {
		auto logCacheSizeIf = [name, actual](uint64_t desired, const char* description) {
			if (actual != desired)
				return;

			CATAPULT_LOG(warning) << name << " cache is " << description << " (size = " << actual << ")";
		};

		// log if cache is filling up (or full)
		logCacheSizeIf(max / 2, "half full");
		logCacheSizeIf(max, "full");
	}
}}
