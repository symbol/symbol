#include "Observers.h"
#include "LockInfoObserverUtils.h"
#include "src/cache/SecretLockInfoCache.h"

namespace catapult { namespace observers {

	using Notification = model::ProofPublicationNotification;

	namespace {
		struct SecretTraits {
		public:
			using CacheType = cache::SecretLockInfoCache;
			using Notification = observers::Notification;

			static auto NotificationToKey(const Notification& notification) {
				return notification.Secret;
			}

			static auto DestinationAccount(const model::SecretLockInfo& lockInfo) {
				return lockInfo.Recipient;
			}
		};
	}

	DEFINE_OBSERVER(Proof, Notification, &LockStatusAccountBalanceObserver<SecretTraits>)
}}
