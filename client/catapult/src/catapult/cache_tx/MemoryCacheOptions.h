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

	/// Options for customizing the behavior of a memory based cache.
	class MemoryCacheOptions {
	public:
		/// Creates default options.
		constexpr MemoryCacheOptions() = default;

		/// Creates options with custom \a maxResponseSize and \a maxCacheSize.
		constexpr MemoryCacheOptions(utils::FileSize maxResponseSize, utils::FileSize maxCacheSize)
				: MaxResponseSize(maxResponseSize)
				, MaxCacheSize(maxCacheSize)
		{}

	public:
		/// Maximum response size.
		utils::FileSize MaxResponseSize;

		/// Maximum size of the cache.
		utils::FileSize MaxCacheSize;
	};
}}
