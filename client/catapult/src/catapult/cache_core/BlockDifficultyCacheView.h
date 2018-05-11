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

namespace catapult { namespace cache {

	/// Mixins used by the block difficulty cache view.
	using BlockDifficultyCacheViewMixins = BasicCacheMixins<
		BlockDifficultyCacheTypes::PrimaryTypes::BaseSetType,
		BlockDifficultyCacheDescriptor>;

	/// Basic view on top of the block difficulty cache.
	class BasicBlockDifficultyCacheView
			: public utils::MoveOnly
			, public BlockDifficultyCacheViewMixins::Size
			, public BlockDifficultyCacheViewMixins::Contains
			, public BlockDifficultyCacheViewMixins::Iteration {
	public:
		using ReadOnlyView = BlockDifficultyCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a difficultyInfoSets and \a options.
		BasicBlockDifficultyCacheView(
				const BlockDifficultyCacheTypes::BaseSets& difficultyInfoSets,
				const BlockDifficultyCacheTypes::Options& options);

	public:
		/// Gets a range object that spans \a count block difficulty infos starting at the specified \a height.
		DifficultyInfoRange difficultyInfos(Height height, size_t count) const;

	private:
		const BlockDifficultyCacheTypes::PrimaryTypes::BaseSetType& m_difficultyInfos;
	};

	/// View on top of the block difficulty cache.
	class BlockDifficultyCacheView : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheView> {
	public:
		/// Creates a view around \a difficultyInfoSets and \a options
		explicit BlockDifficultyCacheView(
				const BlockDifficultyCacheTypes::BaseSets& difficultyInfoSets,
				const BlockDifficultyCacheTypes::Options& options)
				: ReadOnlyViewSupplier(difficultyInfoSets, options)
		{}
	};
}}
