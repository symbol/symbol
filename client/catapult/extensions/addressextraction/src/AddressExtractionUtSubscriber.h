#pragma once
#include "catapult/cache/UtChangeSubscriber.h"
#include <memory>

namespace catapult { namespace model { class NotificationPublisher; } }

namespace catapult { namespace addressextraction {

	/// Creates an address extraction unconfirmed transactions subscriber around a notification publisher (\a pPublisher).
	std::unique_ptr<cache::UtChangeSubscriber> CreateAddressExtractionChangeSubscriber(
			std::unique_ptr<model::NotificationPublisher>&& pPublisher);
}}
