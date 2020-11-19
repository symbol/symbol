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
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkIdentifier.h"

namespace catapult { namespace cache {

	/// Mixins used by the account restriction cache delta.
	using AccountRestrictionCacheDeltaMixins =
		PatriciaTreeCacheMixins<AccountRestrictionCacheTypes::PrimaryTypes::BaseSetDeltaType, AccountRestrictionCacheDescriptor>;

	/// Basic delta on top of the account restriction cache.
	class BasicAccountRestrictionCacheDelta
			: public utils::MoveOnly
			, public AccountRestrictionCacheDeltaMixins::Size
			, public AccountRestrictionCacheDeltaMixins::Contains
			, public AccountRestrictionCacheDeltaMixins::ConstAccessor
			, public AccountRestrictionCacheDeltaMixins::MutableAccessor
			, public AccountRestrictionCacheDeltaMixins::PatriciaTreeDelta
			, public AccountRestrictionCacheDeltaMixins::BasicInsertRemove
			, public AccountRestrictionCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = AccountRestrictionCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a restrictionSets and \a networkIdentifier.
		BasicAccountRestrictionCacheDelta(
				const AccountRestrictionCacheTypes::BaseSetDeltaPointers& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: AccountRestrictionCacheDeltaMixins::Size(*restrictionSets.pPrimary)
				, AccountRestrictionCacheDeltaMixins::Contains(*restrictionSets.pPrimary)
				, AccountRestrictionCacheDeltaMixins::ConstAccessor(*restrictionSets.pPrimary)
				, AccountRestrictionCacheDeltaMixins::MutableAccessor(*restrictionSets.pPrimary)
				, AccountRestrictionCacheDeltaMixins::PatriciaTreeDelta(*restrictionSets.pPrimary, restrictionSets.pPatriciaTree)
				, AccountRestrictionCacheDeltaMixins::BasicInsertRemove(*restrictionSets.pPrimary)
				, AccountRestrictionCacheDeltaMixins::DeltaElements(*restrictionSets.pPrimary)
				, m_pAccountRestrictionEntries(restrictionSets.pPrimary)
				, m_networkIdentifier(networkIdentifier)
		{}

	public:
		using AccountRestrictionCacheDeltaMixins::ConstAccessor::find;
		using AccountRestrictionCacheDeltaMixins::MutableAccessor::find;

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

	private:
		AccountRestrictionCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pAccountRestrictionEntries;
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// Delta on top of the account restriction cache.
	class AccountRestrictionCacheDelta : public ReadOnlyViewSupplier<BasicAccountRestrictionCacheDelta> {
	public:
		/// Creates a delta around \a restrictionSets and \a networkIdentifier.
		AccountRestrictionCacheDelta(
				const AccountRestrictionCacheTypes::BaseSetDeltaPointers& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: ReadOnlyViewSupplier(restrictionSets, networkIdentifier)
		{}
	};
}}
