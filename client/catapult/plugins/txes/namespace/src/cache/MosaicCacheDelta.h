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
#include "MosaicBaseSets.h"
#include "MosaicCacheMixins.h"
#include "MosaicCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache delta.
	struct MosaicCacheDeltaMixins
			: public PatriciaTreeCacheMixins<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType, MosaicCacheDescriptor> {
		using ConstAccessor = ConstAccessorWithAdapter<MosaicCacheTypes::ConstValueAdapter>;
		using MutableAccessor = MutableAccessorWithAdapter<MosaicCacheTypes::MutableValueAdapter>;
		using Touch = HeightBasedTouchMixin<
			typename MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType,
			typename MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaType>;

		using MosaicDeepSize = MosaicDeepSizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType>;
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
			, public MosaicCacheDeltaMixins::Touch
			, public MosaicCacheDeltaMixins::DeltaElements
			, public MosaicCacheDeltaMixins::MosaicDeepSize {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a mosaicSets and \a deepSize.
		BasicMosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets, size_t deepSize);

	public:
		using MosaicCacheDeltaMixins::ConstAccessor::find;
		using MosaicCacheDeltaMixins::MutableAccessor::find;

	public:
		/// Inserts the mosaic \a entry into the cache.
		void insert(const state::MosaicEntry& entry);

		/// Removes the mosaic specified by its \a id from the cache.
		void remove(MosaicId id);

		/// Removes all mosaics associated with namespace \a id from the cache.
		void remove(NamespaceId id);

		/// Prunes the mosaic cache at \a height.
		void prune(Height height);

	private:
		void removeIfEmpty(const state::MosaicHistory& history);

	private:
		MosaicCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pHistoryById;
		MosaicCacheTypes::NamespaceGroupingTypes::BaseSetDeltaPointerType m_pMosaicIdsByNamespaceId;
		MosaicCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType m_pMosaicIdsByExpiryHeight;
	};

	/// Delta on top of the mosaic cache.
	class MosaicCacheDelta : public ReadOnlyViewSupplier<BasicMosaicCacheDelta> {
	public:
		/// Creates a delta around \a mosaicSets and \a deepSize.
		MosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointers& mosaicSets, size_t deepSize)
				: ReadOnlyViewSupplier(mosaicSets, deepSize)
		{}
	};
}}
