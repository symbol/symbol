#pragma once
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes transaction hashes.
	DECLARE_OBSERVER(TransactionHash, model::TransactionNotification)();
}}
