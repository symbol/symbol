#pragma once
#include "src/model/LockInfo.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/DeltaElementsMixin.h"

namespace catapult { namespace cache {

	/// Mixins used by the lock info cache delta.
	template<typename TDescriptor, typename TCacheTypes>
	struct LockInfoCacheDeltaMixins {
		using Size = SizeMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType>;
		using Contains = ContainsMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor>;
		using ConstAccessor = ConstAccessorMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor>;
		using MutableAccessor = MutableAccessorMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor>;
		using ActivePredicate = ActivePredicateMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor>;
		using BasicInsertRemove = BasicInsertRemoveMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor>;
		using Pruning = HeightBasedPruningMixin<
			typename TCacheTypes::PrimaryTypes::BaseSetDeltaType,
			typename TCacheTypes::HeightGroupingTypes::BaseSetDeltaType>;
		using DeltaElements = deltaset::DeltaElementsMixin<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class BasicLockInfoCacheDelta
			: public utils::MoveOnly
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Size
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Contains
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ActivePredicate
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Pruning
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::DeltaElements {
	public:
		using ReadOnlyView = typename TCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta based on the specified \a lockInfoSets.
		explicit BasicLockInfoCacheDelta(const typename TCacheTypes::BaseSetDeltaPointerType& lockInfoSets)
				: LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Size(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Contains(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ActivePredicate(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Pruning(*lockInfoSets.pPrimary, *lockInfoSets.pHeightGrouping)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::DeltaElements(*lockInfoSets.pPrimary)
				, m_pDelta(lockInfoSets.pPrimary)
				, m_pHeightGroupingDelta(lockInfoSets.pHeightGrouping)
		{}

	public:
		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor::get;
		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor::get;

		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor::tryGet;
		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor::tryGet;

	public:
		/// Inserts \a value into the cache.
		void insert(const typename TDescriptor::ValueType& value) {
			LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove::insert(value);
			AddIdentifierWithGroup(*m_pHeightGroupingDelta, value.Height, TDescriptor::GetKeyFromValue(value));
		}

		/// Collects all unused lock infos that expired at \a height.
		std::vector<const typename TDescriptor::ValueType*> collectUnusedExpiredLocks(Height height) {
			std::vector<const typename TDescriptor::ValueType*> values;
			ForEachIdentifierWithGroup(utils::as_const(*m_pDelta), *m_pHeightGroupingDelta, height, [this, &values](const auto& lockInfo) {
				if (model::LockStatus::Unused == lockInfo.Status)
					values.push_back(&lockInfo);
			});

			return values;
		}

	private:
		typename TCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pDelta;
		typename TCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType m_pHeightGroupingDelta;
	};

	/// Delta on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class LockInfoCacheDelta : public ReadOnlyViewSupplier<BasicLockInfoCacheDelta<TDescriptor, TCacheTypes>> {
	public:
		/// Creates a delta based on the specified \a lockInfoSets.
		explicit LockInfoCacheDelta(const typename TCacheTypes::BaseSetDeltaPointerType& lockInfoSets)
				: ReadOnlyViewSupplier<BasicLockInfoCacheDelta<TDescriptor, TCacheTypes>>(lockInfoSets)
		{}
	};
}}
