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
#include "Observers.h"
#include "src/model/LockInfo.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	/// On commit, credits the expiration account of expired locks.
	/// On rollback, debits the expiration account of expired locks.
	/// Uses the observer \a context to determine notification direction and access caches.
	/// Uses \a ownerAccountIdSupplier to retrieve the lock owner's account identifier.
	template<typename TLockInfoCache, typename TAccountIdSupplier>
	void HandleExpiredLockInfos(const ObserverContext& context, TAccountIdSupplier ownerAccountIdSupplier) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& lockInfoCache = context.Cache.template sub<TLockInfoCache>();

		auto lockInfos = lockInfoCache.collectUnusedExpiredLocks(context.Height);
		for (const auto* pLockInfo : lockInfos) {
			auto& accountState = accountStateCache.get(ownerAccountIdSupplier(*pLockInfo));
			if (NotifyMode::Commit == context.Mode)
				accountState.Balances.credit(pLockInfo->MosaicId, pLockInfo->Amount);
			else
				accountState.Balances.debit(pLockInfo->MosaicId, pLockInfo->Amount);
		}
	}

	/// On commit, marks lock as used and credits destination account.
	/// On rollback, marks lock as unused and debits destination account.
	/// Uses the observer \a context to determine notification direction and access caches.
	/// Uses \a notification to determine the destination account.
	template<typename TTraits>
	void LockStatusAccountBalanceObserver(const typename TTraits::Notification& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& cache = context.Cache.template sub<typename TTraits::CacheType>();
		const auto& key = TTraits::NotificationToKey(notification);
		auto& lockInfo = cache.get(key);
		auto& account = accountStateCache.get(TTraits::DestinationAccount(lockInfo));

		if (NotifyMode::Commit == context.Mode) {
			cache.get(key).Status = model::LockStatus::Used;
			account.Balances.credit(lockInfo.MosaicId, lockInfo.Amount);
		} else {
			cache.get(key).Status = model::LockStatus::Unused;
			account.Balances.debit(lockInfo.MosaicId, lockInfo.Amount);
		}
	}
}}
