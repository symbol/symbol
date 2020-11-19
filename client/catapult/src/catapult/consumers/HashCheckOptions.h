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
#include <stdint.h>

namespace catapult { namespace consumers {

	/// Options for configuring the hash check consumer.
	struct HashCheckOptions {
	public:
		/// Creates default options.
		constexpr HashCheckOptions() : HashCheckOptions(0, 0, 0)
		{}

		/// Creates options with custom \a cacheDuration, \a pruneInterval and \a maxCacheSize.
		constexpr HashCheckOptions(uint64_t cacheDuration, uint64_t pruneInterval, uint64_t maxCacheSize)
				: CacheDuration(cacheDuration)
				, PruneInterval(pruneInterval)
				, MaxCacheSize(maxCacheSize)
		{}

	public:
		/// Amount of time a hash should be cached.
		uint64_t CacheDuration;

		/// Minimum amount of time between cache pruning.
		uint64_t PruneInterval;

		/// Maximum size of the cache.
		uint64_t MaxCacheSize;
	};
}}
