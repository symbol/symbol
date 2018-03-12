#pragma once
#include "BlockDifficultyCacheDelta.h"
#include "BlockDifficultyCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using BlockDifficultyBasicCache = BasicCache<
		BlockDifficultyCacheDescriptor,
		BlockDifficultyCacheTypes::BaseSetType,
		BlockDifficultyCacheTypes::Options>;

	/// Cache composed of block difficulty information.
	/// \note The ordering of the elements is solely done by comparing the block height contained in the element.
	class BasicBlockDifficultyCache : public BlockDifficultyBasicCache {
	public:
		/// Creates a cache with the specified difficulty history size (\a difficultyHistorySize).
		explicit BasicBlockDifficultyCache(uint64_t difficultyHistorySize)
				: BlockDifficultyBasicCache(BlockDifficultyCacheTypes::Options{ difficultyHistorySize })
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
