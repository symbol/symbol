#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/validators/SequentialValidationPolicy.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace chain {

	/// Configuration for executing entities.
	struct ExecutionConfiguration {
	private:
		using ObserverPointer = std::shared_ptr<const observers::AggregateNotificationObserver>;
		using ValidatorPointer = std::shared_ptr<const validators::stateful::AggregateNotificationValidator>;
		using PublisherPointer = std::shared_ptr<const model::NotificationPublisher>;

	public:
		/// The network info.
		model::NetworkInfo Network;

		/// The observer.
		ObserverPointer pObserver;

		/// The stateful validator.
		ValidatorPointer pValidator;

		/// The notification publisher.
		PublisherPointer pNotificationPublisher;
	};
}}
