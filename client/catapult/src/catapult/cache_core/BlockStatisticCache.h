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
#include "BlockStatisticCacheDelta.h"
#include "BlockStatisticCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using BlockStatisticBasicCache = BasicCache<
		BlockStatisticCacheDescriptor,
		BlockStatisticCacheTypes::BaseSets,
		BlockStatisticCacheTypes::Options>;

	/// Cache composed of block statistic information.
	/// \note The ordering of the elements is solely done by comparing the block height contained in the element.
	class BasicBlockStatisticCache : public BlockStatisticBasicCache {
	public:
		/// Creates a cache with the specified history size (\a historySize).
		explicit BasicBlockStatisticCache(uint64_t historySize)
				// block statistic cache must always be an in-memory cache
				: BlockStatisticBasicCache(CacheConfiguration(), BlockStatisticCacheTypes::Options{ historySize })
		{}
	};

	/// Synchronized cache composed of block statistic information.
	class BlockStatisticCache : public SynchronizedCache<BasicBlockStatisticCache> {
	public:
		DEFINE_CACHE_CONSTANTS(BlockStatistic)

	public:
		/// Creates a cache with the specified history size (\a historySize).
		explicit BlockStatisticCache(uint64_t historySize)
				: SynchronizedCache<BasicBlockStatisticCache>(BasicBlockStatisticCache(historySize))
		{}
	};
}}
