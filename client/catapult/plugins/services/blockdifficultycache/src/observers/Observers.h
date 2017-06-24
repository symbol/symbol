#pragma once
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes block difficulties.
	NotificationObserverPointerT<model::BlockNotification> CreateBlockDifficultyObserver();

	/// Observes blocks and triggers pruning if necessary. Pruning is done every \a pruneInterval blocks.
	NotificationObserverPointerT<model::BlockNotification> CreateBlockDifficultyPruningObserver(size_t pruneInterval);
}}
