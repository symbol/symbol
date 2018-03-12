#include "Observers.h"
#include "LockInfoObserverUtils.h"
#include "src/cache/SecretLockInfoCache.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	DECLARE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification)() {
		return MAKE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification, ([](const auto&, const ObserverContext& context) {
			HandleExpiredLockInfos<cache::SecretLockInfoCache>(context, [](const auto& lockInfo) { return lockInfo.Account; });
		}));
	}
}}
