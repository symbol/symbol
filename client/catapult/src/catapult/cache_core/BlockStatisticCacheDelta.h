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
#include "BlockStatisticCacheMixins.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/BaseSetDeltaIterationView.h"

namespace catapult { namespace cache {

	/// Mixins used by the block statistic cache delta.
	struct BlockStatisticCacheDeltaMixins
			: public BasicCacheMixins<BlockStatisticCacheTypes::PrimaryTypes::BaseSetDeltaType, BlockStatisticCacheDescriptor> {
		using BlockStatisticRange = BlockStatisticRangeMixin<BlockStatisticCacheTypes::PrimaryTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the block statistic cache.
	class BasicBlockStatisticCacheDelta
			: public utils::MoveOnly
			, public BlockStatisticCacheDeltaMixins::Size
			, public BlockStatisticCacheDeltaMixins::Contains
			, public BlockStatisticCacheDeltaMixins::DeltaElements
			, public BlockStatisticCacheDeltaMixins::BlockStatisticRange {
	public:
		using ReadOnlyView = BlockStatisticCacheTypes::CacheReadOnlyType;
		using ValueType = BlockStatisticCacheDescriptor::ValueType;
		using IterableView = IterationMixin<BlockStatisticCacheTypes::PrimaryTypes::BaseSetDeltaType>::IterableView;

	public:
		/// Creates a delta around \a statisticSets and \a options.
		BasicBlockStatisticCacheDelta(
				const BlockStatisticCacheTypes::BaseSetDeltaPointers& statisticSets,
				const BlockStatisticCacheTypes::Options& options);

	public:
		/// Gets the pruning boundary that is used during commit.
		deltaset::PruningBoundary<ValueType> pruningBoundary() const;

	public:
		/// Creates an iterable view of the cache.
		/// \note Match BlockStatisticCacheDeltaMixins::Iteration signature but don't derive because
		///       IsBaseSetIterable is not (and should not be) implemented for base set deltas.
		std::unique_ptr<IterableView> tryMakeIterableView() const;

	public:
		/// Inserts a block \a statistic into the set.
		void insert(const ValueType& statistic);

		/// Removes a block \a statistic from the set.
		void remove(const ValueType& statistic);

		/// Removes a block statistic from the set at \a height.
		void remove(Height height);

	public:
		/// Prunes the cache at \a height.
		void prune(Height height);

	private:
		void checkInsert(Height height);
		void checkRemove(Height height) const;
		Height nextHeight() const;

	private:
		BlockStatisticCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pOrderedDelta;
		uint64_t m_historySize;
		Height m_startHeight;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the block statistic cache.
	class BlockStatisticCacheDelta : public ReadOnlyViewSupplier<BasicBlockStatisticCacheDelta> {
	public:
		/// Creates a delta around \a statisticSets and \a options.
		BlockStatisticCacheDelta(
				const BlockStatisticCacheTypes::BaseSetDeltaPointers& statisticSets,
				const BlockStatisticCacheTypes::Options& options)
				: ReadOnlyViewSupplier(statisticSets, options)
		{}
	};
}}
