#include "Observers.h"
#include "src/cache/HashLockInfoCache.h"
#include "src/model/LockInfo.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	using Notification = model::HashLockNotification;

	namespace {
		auto CreateLockInfo(const Key& account, Height endHeight, const Notification& notification) {
			return model::HashLockInfo(account, notification.Mosaic.MosaicId, notification.Mosaic.Amount, endHeight, notification.Hash);
		}
	}

	DEFINE_OBSERVER(HashLock, Notification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& signer = accountStateCache.get(notification.Signer);
		auto& cache = context.Cache.sub<cache::HashLockInfoCache>();

		if (NotifyMode::Commit == context.Mode) {
			auto endHeight = context.Height + Height(notification.Duration.unwrap());
			cache.insert(CreateLockInfo(signer.PublicKey, endHeight, notification));
			signer.Balances.debit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		} else {
			cache.remove(notification.Hash);
			signer.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		}
	});
}}
