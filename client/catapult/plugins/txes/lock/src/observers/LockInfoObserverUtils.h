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
