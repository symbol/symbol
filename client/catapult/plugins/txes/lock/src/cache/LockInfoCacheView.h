#pragma once
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the lock info cache view.
	template<typename TDescriptor, typename TCacheTypes>
	struct LockInfoCacheViewMixins {
		using Size = SizeMixin<typename TCacheTypes::PrimaryTypes::BaseSetType>;
		using Contains = ContainsMixin<typename TCacheTypes::PrimaryTypes::BaseSetType, TDescriptor>;
		using MapIteration = MapIterationMixin<typename TCacheTypes::PrimaryTypes::BaseSetType, TDescriptor>;
		using ConstAccessor = ConstAccessorMixin<typename TCacheTypes::PrimaryTypes::BaseSetType, TDescriptor>;
		using ActivePredicate = ActivePredicateMixin<typename TCacheTypes::PrimaryTypes::BaseSetType, TDescriptor>;
	};

	/// Basic view on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class BasicLockInfoCacheView
			: public utils::MoveOnly
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Size
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Contains
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::MapIteration
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ConstAccessor
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ActivePredicate {
	public:
		using ReadOnlyView = typename TCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view based on the specified \a lockInfoSets.
		explicit BasicLockInfoCacheView(const typename TCacheTypes::BaseSetType& lockInfoSets)
				: LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Size(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Contains(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::MapIteration(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ConstAccessor(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ActivePredicate(lockInfoSets.Primary)
		{}
	};

	/// View on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class LockInfoCacheView : public ReadOnlyViewSupplier<BasicLockInfoCacheView<TDescriptor, TCacheTypes>> {
	public:
		/// Creates a view around \a lockInfoSets.
		explicit LockInfoCacheView(const typename TCacheTypes::BaseSetType& lockInfoSets)
				: ReadOnlyViewSupplier<BasicLockInfoCacheView<TDescriptor, TCacheTypes>>(lockInfoSets)
		{}
	};
}}
