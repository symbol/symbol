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
#include "BlockDifficultyCacheDelta.h"
#include "BlockDifficultyCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using BlockDifficultyBasicCache = BasicCache<
		BlockDifficultyCacheDescriptor,
		BlockDifficultyCacheTypes::BaseSets,
		BlockDifficultyCacheTypes::Options>;

	/// Cache composed of block difficulty information.
	/// \note The ordering of the elements is solely done by comparing the block height contained in the element.
	class BasicBlockDifficultyCache : public BlockDifficultyBasicCache {
	public:
		/// Creates a cache with the specified difficulty history size (\a difficultyHistorySize).
		explicit BasicBlockDifficultyCache(uint64_t difficultyHistorySize)
				// block difficulty cache must always be an in-memory cache
				: BlockDifficultyBasicCache(CacheConfiguration(), BlockDifficultyCacheTypes::Options{ difficultyHistorySize })
		{}
	};

	/// Synchronized cache composed of block difficulty information.
	class BlockDifficultyCache : public SynchronizedCache<BasicBlockDifficultyCache> {
	public:
		DEFINE_CACHE_CONSTANTS(BlockDifficulty)

	public:
		/// Creates a cache with the specified difficulty history size (\a difficultyHistorySize).
		explicit BlockDifficultyCache(uint64_t difficultyHistorySize)
				: SynchronizedCache<BasicBlockDifficultyCache>(BasicBlockDifficultyCache(difficultyHistorySize))
		{}
	};
}}
