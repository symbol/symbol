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
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"

namespace catapult { namespace cache {

	/// Mixins used by the lock info cache delta.
	template<typename TDescriptor, typename TCacheTypes>
	struct LockInfoCacheDeltaMixins : public PatriciaTreeCacheMixins<typename TCacheTypes::PrimaryTypes::BaseSetDeltaType, TDescriptor> {
		using Pruning = HeightBasedPruningMixin<
			typename TCacheTypes::PrimaryTypes::BaseSetDeltaType,
			typename TCacheTypes::HeightGroupingTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class BasicLockInfoCacheDelta
			: public utils::MoveOnly
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Size
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Contains
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::PatriciaTreeDelta
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ActivePredicate
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Pruning
			, public LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::DeltaElements {
	public:
		using ReadOnlyView = typename TCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a lockInfoSets.
		explicit BasicLockInfoCacheDelta(const typename TCacheTypes::BaseSetDeltaPointers& lockInfoSets)
				: LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Size(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Contains(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::PatriciaTreeDelta(*lockInfoSets.pPrimary, lockInfoSets.pPatriciaTree)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ActivePredicate(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove(*lockInfoSets.pPrimary)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::Pruning(*lockInfoSets.pPrimary, *lockInfoSets.pHeightGrouping)
				, LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::DeltaElements(*lockInfoSets.pPrimary)
				, m_pDelta(lockInfoSets.pPrimary)
				, m_pHeightGroupingDelta(lockInfoSets.pHeightGrouping)
		{}

	public:
		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::ConstAccessor::find;
		using LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::MutableAccessor::find;

	public:
		/// Inserts \a value into the cache.
		void insert(const typename TDescriptor::ValueType& value) {
			LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove::insert(value);
			AddIdentifierWithGroup(*m_pHeightGroupingDelta, value.EndHeight, TDescriptor::GetKeyFromValue(value));
		}

		/// Removes the value identified by \a key from the cache.
		void remove(const typename TDescriptor::KeyType& key) {
			auto iter = m_pDelta->find(key);
			const auto* pLockInfo = iter.get();
			if (!!pLockInfo)
				RemoveIdentifierWithGroup(*m_pHeightGroupingDelta, pLockInfo->EndHeight, key);

			LockInfoCacheDeltaMixins<TDescriptor, TCacheTypes>::BasicInsertRemove::remove(key);
		}

		/// Processes all unused lock infos that expired at \a height by passing them to \a consumer
		void processUnusedExpiredLocks(Height height, const consumer<const typename TDescriptor::ValueType>& consumer) const {
			// use non-const set to touch all affected lock infos so that active to inactive transitions are visible
			ForEachIdentifierWithGroup(*m_pDelta, *m_pHeightGroupingDelta, height, [consumer](const auto& lockInfo) {
				if (state::LockStatus::Unused == lockInfo.Status)
					consumer(lockInfo);
			});
		}

	private:
		typename TCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pDelta;
		typename TCacheTypes::HeightGroupingTypes::BaseSetDeltaPointerType m_pHeightGroupingDelta;
	};

	/// Delta on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes, typename TBasicView = BasicLockInfoCacheDelta<TDescriptor, TCacheTypes>>
	class LockInfoCacheDelta : public ReadOnlyViewSupplier<TBasicView> {
	public:
		/// Creates a delta around \a lockInfoSets.
		explicit LockInfoCacheDelta(const typename TCacheTypes::BaseSetDeltaPointers& lockInfoSets)
				: ReadOnlyViewSupplier<TBasicView>(lockInfoSets)
		{}
	};
}}
