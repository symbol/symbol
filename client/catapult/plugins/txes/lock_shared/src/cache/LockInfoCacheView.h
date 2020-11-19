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
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the lock info cache view.
	template<typename TDescriptor, typename TCacheTypes>
	using LockInfoCacheViewMixins = PatriciaTreeCacheMixins<typename TCacheTypes::PrimaryTypes::BaseSetType, TDescriptor>;

	/// Basic view on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes>
	class BasicLockInfoCacheView
			: public utils::MoveOnly
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Size
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Contains
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Iteration
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ConstAccessor
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::PatriciaTreeView
			, public LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ActivePredicate {
	public:
		using ReadOnlyView = typename TCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a lockInfoSets.
		explicit BasicLockInfoCacheView(const typename TCacheTypes::BaseSets& lockInfoSets)
				: LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Size(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Contains(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::Iteration(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ConstAccessor(lockInfoSets.Primary)
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::PatriciaTreeView(lockInfoSets.PatriciaTree.get())
				, LockInfoCacheViewMixins<TDescriptor, TCacheTypes>::ActivePredicate(lockInfoSets.Primary)
		{}
	};

	/// View on top of the lock info cache.
	template<typename TDescriptor, typename TCacheTypes, typename TBasicView = BasicLockInfoCacheView<TDescriptor, TCacheTypes>>
	class LockInfoCacheView : public ReadOnlyViewSupplier<TBasicView> {
	public:
		/// Creates a view around \a lockInfoSets.
		explicit LockInfoCacheView(const typename TCacheTypes::BaseSets& lockInfoSets)
				: ReadOnlyViewSupplier<TBasicView>(lockInfoSets)
		{}
	};
}}
