#pragma once
#include "BlockDifficultyCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the block difficulty cache.
	class BasicBlockDifficultyCacheDelta
			: public utils::MoveOnly
			, public SizeMixin<BlockDifficultyCacheTypes::BaseSetDeltaType>
			, public ContainsMixin<BlockDifficultyCacheTypes::BaseSetDeltaType, BlockDifficultyCacheDescriptor> {
	public:
		using ReadOnlyView = BlockDifficultyCacheTypes::CacheReadOnlyType;
		using ValueType = BlockDifficultyCacheDescriptor::ValueType;

	public:
		/// Creates a delta based on the block difficulty set (\a pBlockDifficultyInfos) and \a options.
		explicit BasicBlockDifficultyCacheDelta(
				const BlockDifficultyCacheTypes::BaseSetDeltaPointerType& pBlockDifficultyInfos,
				const BlockDifficultyCacheTypes::Options& options);

	public:
		/// Gets the pruning boundary that is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const;

	public:
		/// Inserts a block difficulty \a info into the set.
		void insert(const ValueType& info);

		/// Inserts a block difficulty info into the set given a \a height and a \a timestamp and a \a difficulty.
		void insert(Height height, Timestamp timestamp, Difficulty difficulty);

		/// Removes a block difficulty \a info from the set.
		void remove(const ValueType& info);

		/// Removes a block difficulty info from the set given a \a height.
		void remove(Height height);

	public:
		/// Removes all block difficulty infos that have a height less than the given height
		/// minus a constant (constant = rewrite limit + 60).
		void prune(Height height);

	private:
		void checkInsert(Height height);
		void checkRemove(Height height) const;
		Height nextHeight() const;

	private:
		BlockDifficultyCacheTypes::BaseSetDeltaPointerType m_pOrderedDelta;
		uint64_t m_difficultyHistorySize;
		Height m_startHeight;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the block difficulty cache.
	class BlockDifficultyCacheDelta : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheDelta> {
	public:
		/// Creates a delta based on the block difficulty set (\a pBlockDifficultyInfos) and \a options.
		explicit BlockDifficultyCacheDelta(
				const BlockDifficultyCacheTypes::BaseSetDeltaPointerType& pBlockDifficultyInfos,
				const BlockDifficultyCacheTypes::Options& options)
				: ReadOnlyViewSupplier(pBlockDifficultyInfos, options)
		{}
	};
}}
