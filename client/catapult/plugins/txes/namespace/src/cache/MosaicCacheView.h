#pragma once
#include "MosaicCacheMixins.h"
#include "MosaicCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic cache view.
	struct MosaicCacheViewMixins {
		using Size = SizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType>;
		using Contains = ContainsMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType, MosaicCacheDescriptor>;
		using MapIteration = MapIterationMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType, MosaicCacheDescriptor>;
		using ConstAccessor = ConstAccessorMixin<
			MosaicCacheTypes::PrimaryTypes::BaseSetType,
			MosaicCacheDescriptor,
			MosaicCacheTypes::ConstValueAdapter>;
		using ActivePredicate = ActivePredicateMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType, MosaicCacheDescriptor>;

		using MosaicDeepSize = MosaicDeepSizeMixin<MosaicCacheTypes::PrimaryTypes::BaseSetType>;
	};

	/// Basic view on top of the mosaic cache.
	class BasicMosaicCacheView
			: public utils::MoveOnly
			, public MosaicCacheViewMixins::Size
			, public MosaicCacheViewMixins::Contains
			, public MosaicCacheViewMixins::MapIteration
			, public MosaicCacheViewMixins::ConstAccessor
			, public MosaicCacheViewMixins::ActivePredicate
			, public MosaicCacheViewMixins::MosaicDeepSize {
	public:
		using ReadOnlyView = MosaicCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a mosaicSets.
		explicit BasicMosaicCacheView(const MosaicCacheTypes::BaseSetType& mosaicSets)
				: MosaicCacheViewMixins::Size(mosaicSets.Primary)
				, MosaicCacheViewMixins::Contains(mosaicSets.Primary)
				, MosaicCacheViewMixins::MapIteration(mosaicSets.Primary)
				, MosaicCacheViewMixins::ConstAccessor(mosaicSets.Primary)
				, MosaicCacheViewMixins::ActivePredicate(mosaicSets.Primary)
				, MosaicCacheViewMixins::MosaicDeepSize(mosaicSets.Primary)
		{}
	};

	/// View on top of the mosaic cache.
	class MosaicCacheView : public ReadOnlyViewSupplier<BasicMosaicCacheView> {
	public:
		/// Creates a view around \a mosaicSets.
		explicit MosaicCacheView(const MosaicCacheTypes::BaseSetType& mosaicSets)
				: ReadOnlyViewSupplier(mosaicSets)
		{}
	};
}}
