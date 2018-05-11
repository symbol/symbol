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
#include "BlockDifficultyCacheTypes.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Mixins used by the block difficulty cache delta.
	using BlockDifficultyCacheDeltaMixins = BasicCacheMixins<
		BlockDifficultyCacheTypes::PrimaryTypes::BaseSetDeltaType,
		BlockDifficultyCacheDescriptor>;

	/// Basic delta on top of the block difficulty cache.
	class BasicBlockDifficultyCacheDelta
			: public utils::MoveOnly
			, public BlockDifficultyCacheDeltaMixins::Size
			, public BlockDifficultyCacheDeltaMixins::Contains {
	public:
		using ReadOnlyView = BlockDifficultyCacheTypes::CacheReadOnlyType;
		using ValueType = BlockDifficultyCacheDescriptor::ValueType;

	public:
		/// Creates a delta around \a difficultyInfoSets and \a options.
		explicit BasicBlockDifficultyCacheDelta(
				const BlockDifficultyCacheTypes::BaseSetDeltaPointers& difficultyInfoSets,
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
		BlockDifficultyCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pOrderedDelta;
		uint64_t m_difficultyHistorySize;
		Height m_startHeight;
		deltaset::PruningBoundary<ValueType> m_pruningBoundary;
	};

	/// Delta on top of the block difficulty cache.
	class BlockDifficultyCacheDelta : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheDelta> {
	public:
		/// Creates a delta around \a difficultyInfoSets and \a options.
		explicit BlockDifficultyCacheDelta(
				const BlockDifficultyCacheTypes::BaseSetDeltaPointers& difficultyInfoSets,
				const BlockDifficultyCacheTypes::Options& options)
				: ReadOnlyViewSupplier(difficultyInfoSets, options)
		{}
	};
}}
