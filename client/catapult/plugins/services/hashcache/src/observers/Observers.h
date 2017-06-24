#pragma once
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes transaction hashes.
	NotificationObserverPointerT<model::TransactionNotification> CreateTransactionHashObserver();

	/// Observes blocks and triggers pruning if necessary. Pruning is done every \a pruneInterval blocks.
	NotificationObserverPointerT<model::BlockNotification> CreateTransactionHashPruningObserver(size_t pruneInterval);
}}
