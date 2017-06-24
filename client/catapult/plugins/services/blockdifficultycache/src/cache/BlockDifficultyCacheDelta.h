#pragma once
#include "BlockDifficultyCacheTypes.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the block difficulty cache.
	class BasicBlockDifficultyCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = block_difficulty_cache_types::CacheReadOnlyType;
		using ValueType = block_difficulty_cache_types::ValueType;

	public:
		/// Creates a delta with the specified original element (\a pFirstOriginalElement)
		/// and difficulty history size (\a difficultyHistorySize) based on the specified delta (\a pDelta).
		explicit BasicBlockDifficultyCacheDelta(
				const block_difficulty_cache_types::BaseSetDeltaPointerType& pDelta,
				const ValueType* const pFirstOriginalElement,
				uint64_t difficultyHistorySize);

	public:
		/// Gets the size of the set.
		size_t size() const {
			return m_pOrderedDelta->size();
		}

		/// Searches for the given block difficulty \a info in the set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const ValueType& info) const {
			return m_pOrderedDelta->contains(info);
		}

		/// Inserts a block difficulty \a info into the set.
		void insert(const ValueType& info) {
			checkInsert(info.BlockHeight);
			m_pOrderedDelta->insert(info);
		}

		/// Inserts a block difficulty info into the set given a \a height and a \a timestamp and a \a difficulty.
		void insert(Height height, Timestamp timestamp, Difficulty difficulty) {
			checkInsert(height);
			m_pOrderedDelta->emplace(height, timestamp, difficulty);
		}

		/// Removes a block difficulty \a info from the set.
		void remove(const ValueType& info) {
			checkRemove(info.BlockHeight);
			remove(info.BlockHeight);
		}

		/// Removes a block difficulty info from the set given a \a height.
		void remove(Height height) {
			checkRemove(height);
			m_pOrderedDelta->remove(ValueType(height));
		}

	public:
		/// Removes all block difficulty infos that have a height less than the given height
		/// minus a constant (constant = rewrite limit + 60).
		void prune(Height height);

		/// Gets the pruning boundary which is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const {
			return m_pruningBoundary;
		}

	private:
		void checkInsert(Height height);
		void checkRemove(Height height) const;
		Height nextHeight() const;

	private:
		block_difficulty_cache_types::BaseSetDeltaPointerType m_pOrderedDelta;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
		uint64_t m_difficultyHistorySize;
		Height m_startHeight;
	};

	/// Delta on top of the block difficulty cache.
	class BlockDifficultyCacheDelta : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheDelta> {
	public:
		/// Creates a delta with the specified original element (\a pFirstOriginalElement)
		/// and difficulty history size (\a difficultyHistorySize) based on the specified delta (\a pDelta).
		explicit BlockDifficultyCacheDelta(
				const block_difficulty_cache_types::BaseSetDeltaPointerType& pDelta,
				const ValueType* const pFirstOriginalElement,
				uint64_t difficultyHistorySize)
				: ReadOnlyViewSupplier(pDelta, pFirstOriginalElement, difficultyHistorySize)
		{}
	};
}}
