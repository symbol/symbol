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
#include "MosaicBaseSets.h"
#include "MosaicCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache view.
	struct MosaicCacheViewMixins : public PatriciaTreeCacheMixins<MosaicCacheTypes::PrimaryTypes::BaseSetType, MosaicCacheDescriptor> {};

	/// Basic view on top of the mosaic cache.
	class BasicMosaicCacheView
			: public utils::MoveOnly
			, public MosaicCacheViewMixins::Size
			, public MosaicCacheViewMixins::Contains
			, public MosaicCacheViewMixins::Iteration
			, public MosaicCacheViewMixins::ConstAccessor
			, public MosaicCacheDeltaMixins::PatriciaTreeView
			, public MosaicCacheViewMixins::ActivePredicate {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a mosaicSets.
		explicit BasicMosaicCacheView(const MosaicCacheTypes::BaseSets& mosaicSets)
				: MosaicCacheViewMixins::Size(mosaicSets.Primary)
				, MosaicCacheViewMixins::Contains(mosaicSets.Primary)
				, MosaicCacheViewMixins::Iteration(mosaicSets.Primary)
				, MosaicCacheViewMixins::ConstAccessor(mosaicSets.Primary)
				, MosaicCacheViewMixins::PatriciaTreeView(mosaicSets.PatriciaTree.get())
				, MosaicCacheViewMixins::ActivePredicate(mosaicSets.Primary)
		{}
	};

	/// View on top of the mosaic cache.
	class MosaicCacheView : public ReadOnlyViewSupplier<BasicMosaicCacheView> {
	public:
		/// Creates a view around \a mosaicSets.
		explicit MosaicCacheView(const MosaicCacheTypes::BaseSets& mosaicSets) : ReadOnlyViewSupplier(mosaicSets)
		{}
	};
}}
