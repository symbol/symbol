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
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache delta.
	struct MosaicCacheDeltaMixins
			: public PatriciaTreeCacheMixins<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType, MosaicCacheDescriptor> {
		using Touch = HeightBasedTouchMixin<
			typename MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType,
			typename MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the mosaic cache.
	class BasicMosaicCacheDelta
			: public utils::MoveOnly
			, public MosaicCacheDeltaMixins::Size
			, public MosaicCacheDeltaMixins::Contains
			, public MosaicCacheDeltaMixins::ConstAccessor
			, public MosaicCacheDeltaMixins::MutableAccessor
			, public MosaicCacheDeltaMixins::PatriciaTreeDelta
			, public MosaicCacheDeltaMixins::ActivePredicate
			, public MosaicCacheDeltaMixins::BasicInsertRemove
			, public MosaicCacheDeltaMixins::Touch
			, public MosaicCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a mosaicSets.
		explicit BasicMosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets);

	public:
		using MosaicCacheDeltaMixins::ConstAccessor::find;
		using MosaicCacheDeltaMixins::MutableAccessor::find;

	public:
		/// Inserts the mosaic \a entry into the cache.
		void insert(const state::MosaicEntry& entry);

		/// Removes the value identified by \a mosaicId from the cache.
		void remove(MosaicId mosaicId);

	private:
		MosaicCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pEntryById;
		MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType m_pMosaicIdsByExpiryHeight;
	};

	/// Delta on top of the mosaic cache.
	class MosaicCacheDelta : public ReadOnlyViewSupplier<BasicMosaicCacheDelta> {
	public:
		/// Creates a delta around \a mosaicSets.
		explicit MosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets) : ReadOnlyViewSupplier(mosaicSets)
		{}
	};
}}
