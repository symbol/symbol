#pragma once
#include "BlockDifficultyCacheDelta.h"
#include "BlockDifficultyCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of block difficulty information.
	/// \note The ordering of the entities is solely done by comparing the block height contained in the entity.
	class BasicBlockDifficultyCache : public utils::MoveOnly {
	public:
		using CacheViewType = BlockDifficultyCacheView;
		using CacheDeltaType = BlockDifficultyCacheDelta;
		using CacheReadOnlyType = block_difficulty_cache_types::CacheReadOnlyType;

	public:
		/// Creates a block difficulty cache with the specified difficulty history size (\a difficultyHistorySize).
		explicit BasicBlockDifficultyCache(uint64_t difficultyHistorySize)
				: m_difficultyHistorySize(difficultyHistorySize)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_difficultyInfos);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_difficultyInfos.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(m_difficultyInfos.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType& delta) {
			m_difficultyInfos.commit(delta.pruningBoundary());
		}

	private:
		inline CacheDeltaType createDelta(const block_difficulty_cache_types::BaseSetDeltaPointerType& pDelta) const {
			auto pFirstOriginalElement = m_difficultyInfos.empty() ? nullptr : &(*m_difficultyInfos.cbegin());
			return CacheDeltaType(pDelta, pFirstOriginalElement, m_difficultyHistorySize);
		}

	private:
		block_difficulty_cache_types::BaseSetType m_difficultyInfos;
		uint64_t m_difficultyHistorySize;
	};

	/// Synchronized cache composed of block difficulty information.
	class BlockDifficultyCache : public SynchronizedCache<BasicBlockDifficultyCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 1;

		/// The cache friendly name.
		static constexpr auto Name = "BlockDifficultyCache";

	public:
		/// Creates a block difficulty cache with the specified difficulty history size (\a difficultyHistorySize).
		explicit BlockDifficultyCache(uint64_t difficultyHistorySize)
				: SynchronizedCache<BasicBlockDifficultyCache>(BasicBlockDifficultyCache(difficultyHistorySize))
		{}
	};
}}
