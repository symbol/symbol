#include "Observers.h"
#include "src/cache/SecretLockInfoCache.h"
#include "src/model/LockInfo.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	using Notification = model::SecretLockNotification;

	namespace {
		auto CreateLockInfo(const Key& account, Height endHeight, const Notification& notification) {
			return model::SecretLockInfo(
					account,
					notification.Mosaic.MosaicId,
					notification.Mosaic.Amount,
					endHeight,
					notification.HashAlgorithm,
					notification.Secret,
					notification.Recipient);
		}
	}

	DEFINE_OBSERVER(SecretLock, Notification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& signer = accountStateCache.get(notification.Signer);
		auto& cache = context.Cache.sub<cache::SecretLockInfoCache>();

		if (NotifyMode::Commit == context.Mode) {
			auto endHeight = context.Height + Height(notification.Duration.unwrap());
			cache.insert(CreateLockInfo(signer.PublicKey, endHeight, notification));
			signer.Balances.debit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		} else {
			cache.remove(notification.Secret);
			signer.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		}
	});
}}
