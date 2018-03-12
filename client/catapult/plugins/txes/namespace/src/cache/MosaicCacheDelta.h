#pragma once
#include "MosaicCacheMixins.h"
#include "MosaicCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/DeltaElementsMixin.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache delta.
	struct MosaicCacheDeltaMixins {
		using Size = SizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType>;
		using Contains = ContainsMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType, MosaicCacheDescriptor>;
		using ConstAccessor = ConstAccessorMixin<
			MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType,
			MosaicCacheDescriptor,
			MosaicCacheTypes::ConstValueAdapter>;
		using MutableAccessor = MutableAccessorMixin<
			MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType,
			MosaicCacheDescriptor,
			MosaicCacheTypes::MutableValueAdapter>;
		using ActivePredicate = ActivePredicateMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType, MosaicCacheDescriptor>;
		using DeltaElements = deltaset::DeltaElementsMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType>;

		using MosaicDeepSize = MosaicDeepSizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the mosaic cache.
	class BasicMosaicCacheDelta
			: public utils::MoveOnly
			, public MosaicCacheDeltaMixins::Size
			, public MosaicCacheDeltaMixins::Contains
			, public MosaicCacheDeltaMixins::ConstAccessor
			, public MosaicCacheDeltaMixins::MutableAccessor
			, public MosaicCacheDeltaMixins::ActivePredicate
			, public MosaicCacheDeltaMixins::DeltaElements
			, public MosaicCacheDeltaMixins::MosaicDeepSize {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a mosaicSets.
		explicit BasicMosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointerType& mosaicSets);

	public:
		using MosaicCacheDeltaMixins::ConstAccessor::get;
		using MosaicCacheDeltaMixins::MutableAccessor::get;

		using MosaicCacheDeltaMixins::ConstAccessor::tryGet;
		using MosaicCacheDeltaMixins::MutableAccessor::tryGet;

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
		/// Creates a delta around \a mosaicSets.
		explicit MosaicCacheDelta(const MosaicCacheTypes::BaseSetDeltaPointerType& mosaicSets)
				: ReadOnlyViewSupplier(mosaicSets)
		{}
	};
}}
