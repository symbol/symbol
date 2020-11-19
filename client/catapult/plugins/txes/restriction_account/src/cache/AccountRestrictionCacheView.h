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
#include "AccountRestrictionBaseSets.h"
#include "AccountRestrictionCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the account restriction cache view.
	using AccountRestrictionCacheViewMixins =
		PatriciaTreeCacheMixins<AccountRestrictionCacheTypes::PrimaryTypes::BaseSetType, AccountRestrictionCacheDescriptor>;

	/// Basic view on top of the account restriction cache.
	class BasicAccountRestrictionCacheView
			: public utils::MoveOnly
			, public AccountRestrictionCacheViewMixins::Size
			, public AccountRestrictionCacheViewMixins::Contains
			, public AccountRestrictionCacheViewMixins::Iteration
			, public AccountRestrictionCacheViewMixins::ConstAccessor
			, public AccountRestrictionCacheViewMixins::PatriciaTreeView {
	public:
		using ReadOnlyView = AccountRestrictionCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a restrictionSets and \a networkIdentifier.
		BasicAccountRestrictionCacheView(
				const AccountRestrictionCacheTypes::BaseSets& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: AccountRestrictionCacheViewMixins::Size(restrictionSets.Primary)
				, AccountRestrictionCacheViewMixins::Contains(restrictionSets.Primary)
				, AccountRestrictionCacheViewMixins::Iteration(restrictionSets.Primary)
				, AccountRestrictionCacheViewMixins::ConstAccessor(restrictionSets.Primary)
				, AccountRestrictionCacheViewMixins::PatriciaTreeView(restrictionSets.PatriciaTree.get())
				, m_networkIdentifier(networkIdentifier)
		{}

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// View on top of the account restriction cache.
	class AccountRestrictionCacheView : public ReadOnlyViewSupplier<BasicAccountRestrictionCacheView> {
	public:
		/// Creates a view around \a restrictionSets and \a networkIdentifier.
		AccountRestrictionCacheView(
				const AccountRestrictionCacheTypes::BaseSets& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: ReadOnlyViewSupplier(restrictionSets, networkIdentifier)
		{}
	};
}}
