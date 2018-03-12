#pragma once
#include "ContainerTypes.h"

namespace catapult {
	namespace model {
		class NotificationPublisher;
		struct Transaction;
	}
}

namespace catapult { namespace model {

	/// Extracts all addresses that are involved in \a transaction using \a notificationPublisher.
	model::AddressSet ExtractAddresses(const Transaction& transaction, const NotificationPublisher& notificationPublisher);
}}
