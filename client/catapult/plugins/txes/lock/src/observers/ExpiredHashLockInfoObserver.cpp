#include "Observers.h"
#include "LockInfoObserverUtils.h"
#include "src/cache/HashLockInfoCache.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	DECLARE_OBSERVER(ExpiredHashLockInfo, model::BlockNotification)() {
		return MAKE_OBSERVER(ExpiredHashLockInfo, model::BlockNotification, ([](const auto& notification, const ObserverContext& context) {
			HandleExpiredLockInfos<cache::HashLockInfoCache>(context, [&notification](const auto&) { return notification.Signer; });
		}));
	}
}}
