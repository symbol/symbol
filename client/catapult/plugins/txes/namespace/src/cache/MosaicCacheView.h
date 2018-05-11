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
#include "MosaicCacheMixins.h"
#include "MosaicCacheTypes.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache view.
	struct MosaicCacheViewMixins : public BasicCacheMixins<MosaicCacheTypes::PrimaryTypes::BaseSetType, MosaicCacheDescriptor> {
		using ConstAccessor = ConstAccessorWithAdapter<MosaicCacheTypes::ConstValueAdapter>;

		using MosaicDeepSize = MosaicDeepSizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType>;
	};

	/// Basic view on top of the mosaic cache.
	class BasicMosaicCacheView
			: public utils::MoveOnly
			, public MosaicCacheViewMixins::Size
			, public MosaicCacheViewMixins::Contains
			, public MosaicCacheViewMixins::Iteration
			, public MosaicCacheViewMixins::ConstAccessor
			, public MosaicCacheViewMixins::ActivePredicate
			, public MosaicCacheViewMixins::MosaicDeepSize {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a mosaicSets and \a deepSize.
		BasicMosaicCacheView(const MosaicCacheTypes::BaseSets& mosaicSets, size_t deepSize)
				: MosaicCacheViewMixins::Size(mosaicSets.Primary)
				, MosaicCacheViewMixins::Contains(mosaicSets.Primary)
				, MosaicCacheViewMixins::Iteration(mosaicSets.Primary)
				, MosaicCacheViewMixins::ConstAccessor(mosaicSets.Primary)
				, MosaicCacheViewMixins::ActivePredicate(mosaicSets.Primary)
				, MosaicCacheViewMixins::MosaicDeepSize(deepSize)
		{}
	};

	/// View on top of the mosaic cache.
	class MosaicCacheView : public ReadOnlyViewSupplier<BasicMosaicCacheView> {
	public:
		/// Creates a view around \a mosaicSets and \a deepSize.
		MosaicCacheView(const MosaicCacheTypes::BaseSets& mosaicSets, size_t deepSize)
				: ReadOnlyViewSupplier(mosaicSets, deepSize)
		{}
	};
}}
